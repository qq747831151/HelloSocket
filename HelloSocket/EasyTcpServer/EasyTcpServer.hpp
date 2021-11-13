#ifndef _EasyTcpServer_HPP
#define  _EasyTcpServer_HPP
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN//不影响 windows.h 和 WinSock2.h 前后顺序 
#define _WINSOCK_DEPRECATED_NO_WARNINGS //这个用于 inet_ntoa   可以在右击项目属性 C/C++ 预处理里面 预处理定义添加
#include <windows.h>
#include <WinSock2.h>
/*为了可以在其他平台也可以使用 右键项目属性 选择链接器 附加依赖项 将ws2_32.lib 添加进去就行 这样就不需要 下面这些 */
#pragma  comment(lib,"ws2_32.lib")
#else
#include<unistd.h> //uni std
#include<arpa/inet.h>
#include<string.h>
#include<sys/select.h>
#include<pthread.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "MessAgeHeader.hpp"
class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<SOCKET> g_clients;
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;//无效地址
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}
	//初始化
	void InitSocket()
	{
#ifdef _WIN32
		/*启动socket网络环境 2.x环境*/
		WORD ver = MAKEWORD(2, 2);//版本号
		WSADATA dat;
		WSAStartup(ver, &dat);//动态库需要写上那个lib
#endif

		if (_sock!=INVALID_SOCKET)
		{
			printf("<Socket=%d>关闭之前的旧链接\n", _sock);
		}
	//1.建立Socket API 建立简易TC服务端
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (_sock == INVALID_SOCKET)
		{
			printf("ERROR,<socket=%d>建立socket失败...\n", _sock);
		}
		else
		{
			printf("TURE,<socket=%d>建立socket成功.....\n", _sock);
		}

	}
	int Bind(const char *ip,unsigned short port)
	{

		//2.绑定 用于接受客户端连接的网络端口
		sockaddr_in _sin = {  };
		_sin.sin_family = AF_INET;//ipv4
		_sin.sin_port = htons(port);//套字节序应该是网络字节序
#ifdef _WIN32
		if (ip)
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);;
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = ADDR_ANY;//获取IP得操作交给内核
		}
	
	//_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");如果项目只是内网使用的话可以使用127
#else
		if (ip)
		{
			inet_pton(AF_INET, ip, &_sin.sin_addr.s_addr);
		}
		else
		{
			_sin.sin_addr.s_addr = ADDR_ANY;
		}
	//inet_pton(AF_INET, "192.168.17.1", &_sin.sin_addr.s_addr);
		
#endif

		int ret = bind(_sock, (struct sockaddr*)&_sin, sizeof(_sin));
		if (ret == SOCKET_ERROR)
		{
			printf("ERROR,绑定用于接受客户端连接的网络端口失败...\n");
		}
		else
		{
			printf("TURE,绑定用于接受客户端连接的网络端口成功.....\n");
		}
		return ret;
	}
	//监听端口号
	int Listen(int n)
	{
		//3.监听网络端口
		int ret = listen(_sock, n);
		if (ret == SOCKET_ERROR)
		{
			printf("ERROR,监听网络端口失败...\n");
		}
		else
		{
			printf("TURE,监听网络端口成功.....\n");
		}
		return ret;
	}
	//接收客户端连接
	SOCKET Accept()
	{
		//4.等待接收客户端连接
		sockaddr_in addCli = {};
		int nlen = sizeof(sockaddr_in);
		SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
		_cSock = accept(_sock, (sockaddr*)&addCli, &nlen);
#else
		_cSock = accept(_sock, (sockaddr*)&addCli, (socklen_t*)&nlen);
#endif
		if (_cSock==INVALID_SOCKET)
		{
			printf("ERROR,等待接收客户端连接失败\n");
		}
		else
		{
			//如果有新客户端加入,就向其他现有的客户端发送信息
			LoginNewUser newUser;
			SendData2All(&newUser);
			printf("新客户端加入:socket=%d IP=%s\n", (int)_cSock, inet_ntoa(addCli.sin_addr));
			g_clients.push_back(_cSock);
		}
		return _cSock;
	}
	void Close()
	{
#ifdef _WIN32
		for (int i = g_clients.size() - 1; i >= 0; i--)
		{
			closesocket(g_clients[i]);
		}
		//8 关闭套接字
		closesocket(_sock);
		WSACleanup();
#else
		for (int i = g_clients.size() - 1; i >= 0; i--)
		{
			close(g_clients[i]);
		}
		//8 关闭套接字
		close(_sock);
#endif
	}
	//处理网络消息
	int nCount = 0;
	bool OnRun()
	{
		if (IsRun())
		{
			//伯克利 socket
			fd_set fdRead;
			fd_set fdWrite;
			fd_set fdExp;
			//清除集合
			FD_ZERO(&fdRead);// fd_set 共有1024bit, 全部初始化为0
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);
			//加入集合
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);//将参数文件描述符fd对应的标志位,设置为1
			SOCKET maxSock = _sock;
			for (int i = g_clients.size() - 1; i >= 0; i--)
			{
				FD_SET(g_clients[i], &fdRead);//可以放只读 另外两个也可以放
				if (g_clients[i] > maxSock)
				{
					maxSock = g_clients[i];
				}
			}
			struct timeval _time;
			_time.tv_sec = 1;
			_time.tv_usec = 0;
			//nfds 是一个整数值 是指fd_set集合中所有的描述符socket 的范围,而不是数量,
	//既是所有文件描述符最大值+1在windows无所谓 在linux是这样的
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &_time);
			if (ret < 0)
			{
				printf("客户端已退出,任务结束\n");
				Close();
				return false;
			}
			// 判断fd对应的标志位到底是0还是1, 返回值: fd对应的标志位的值, 0, 返回0, 1->返回1
			//有新连接
			//判断描述符(socket)是否在集合中
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);// 将参数文件描述符fd对应的标志位, 设置为0
				Accept();
			}
			//通信,有客户端发送数据过来
			for (int i = g_clients.size() - 1; i >= 0; i--)
			{
				if (FD_ISSET(g_clients[i], &fdRead))
				{
					if (-1 == RecvData(g_clients[i]))
					{
						std::vector<SOCKET>::iterator iter = g_clients.begin() + i;;//i是删除的迭代器
						if (iter != g_clients.end())
						{
							g_clients.erase(iter);//删除
						}
					}
				}
			}
			return true;
		}
		return false;
	}
	/*是否工作中*/
	bool IsRun() {
		return _sock != INVALID_SOCKET;
	}
	//缓冲区
	char szRecv[409600] = {};
	//接受数据 处理粘包 拆分包
	int RecvData(SOCKET clientSock)
	{
		//5.接受客户端请求数据
		//数据存到szRecv中     第三个参数是可接收数据的最大长度
		int nlen = recv(clientSock, szRecv, 409600, 0);//返回值是接收的长度  revcz在mac返回值是long 建议强转int
		if (nlen <= 0)
		{
			printf("客户端<socket=%d>退出,任务结束\n", clientSock);
			return -1;
		}
		LoginResult loginresult;
		SendData(&loginresult, clientSock);

		/*DataHeader* header = (DataHeader*)szRecv;
		recv(clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader),0);
		OnNetMsg(header,clientSock);*/
		return 0;
	}
	//响应网络消息
	virtual void OnNetMsg(DataHeader*header,SOCKET clientSock)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;
			printf("收到的命令:%d 数据长度%d userName:%s password:%s\n", login->cmd, login->dataLength, login->userName, login->passWord);
			//忽略 判断用户名密码是否正确
			LoginResult loginresult;
			send(clientSock, (char*)&loginresult, sizeof(LoginResult), 0);
		}
		break;
		case  CMD_LOGINOUT:
		{
		
			LoginOut* loginout = (LoginOut*)header;
			printf("收到的命令:%d 数据长度%d userName:%s\n", loginout->cmd, loginout->dataLength, loginout->userName);
			//忽略 判断用户名密码是否正确
			LoginOutResult loginOutresult;
			send(clientSock, (char*)&loginOutresult, sizeof(LoginOutResult), 0);
		}
		break;
		default:
			DataHeader dp = { CMD_ERROR,0 };
			send(clientSock, (char*)&dp, sizeof(DataHeader), 0);
			break;
		}
	}
	//发送单发指定的Socket
	int SendData(DataHeader*header,SOCKET clientSock)
	{
		if (IsRun()&&header!=nullptr)
		{
			return send(clientSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	//发送群发数据
	void SendData2All(DataHeader* header)
	{
		if (IsRun() && header != NULL)
		{
			//如果有新客户端加入 就向其他现有的客户端发送
			for (int n = g_clients.size() - 1; n >= 0; n--)
			{
				SendData(header, g_clients[n]);
			}

		}
	}

private:

};
#endif
