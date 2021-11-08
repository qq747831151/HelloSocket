#ifndef _EasyTcpClient_Hpp
#define  _EasyTcpClient_Hpp


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN

#define _WINSOCK_DEPRECATED_NO_WARNINGS //影响inet_addr
#include <windows.h>
#include <WINSock2.h>

#else

#include<unistd.h> //uni std
#include<arpa/inet.h>
#include<string.h>
#include<sys/select.h>
#include<pthread.h>

#define  SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR  (-1)
#endif



#include <stdio.h>
#include <thread>
#include "MessAgeHeader.hpp"

/*为了可以在其他平台也可以使用 右键项目属性 选择链接器 附加依赖项 将ws2_32.lib 添加进去就行 这样就不需要 下面这些 */
#pragma  comment(lib,"ws2_32.lib")
class EasyTcpClient
{
	SOCKET _sock;
public:
	EasyTcpClient() 
	{
		_sock = INVALID_SOCKET;//设置为空
	}
	//虚析构函数
	virtual ~EasyTcpClient() 
	{
		Close();
	}
	void InitSocket()
	{
#ifdef _WIN32
		/*启动socket网络环境 2.x环境*/
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		/*编写*/
	//--用Socket API建立简易的TCP客户端
	//1.建立一个socket套接字 
		if (_sock!=INVALID_SOCKET)
		{
			printf("<socket=%d>关闭之前的旧链接\n", _sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (_sock == INVALID_SOCKET)
		{
			printf("ERROR,<Socket=%d>建立套接字失败\n",_sock);
		}
		else
		{
			printf("TRUE,<Socket=%d>建立套接字成功\n",_sock);
		}
	}
	int Connect(const char *ip,unsigned short port )
	{
		//2.连接服务器 connect
		sockaddr_in _sin;
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);//这个是连接服务端的IP地址
#else
		inet_pton(AF_INET, "192.168.17.1", &_sin.sin_addr.s_addr);
#endif
		int ret = connect(_sock, (struct sockaddr*)&_sin, sizeof(_sin));
		if (ret == INVALID_SOCKET)
		{
			printf("ERROR,连接服务器connect失败........\n");
		}
		else
		{
			printf("TRUE,建连接服务器connect成功.......\n");
		}
		return ret;
	}
	void Close()
	{
		if (_sock != INVALID_SOCKET) 
		{
#ifdef _WIN32
			//7.关闭套接字
			closesocket(_sock);
			WSACleanup();
#else
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		}

	}
	//查询网络消息
	bool OnRun()
	{
		if (IsRun())
		{
			fd_set fdRead;
			FD_ZERO(&fdRead);
			FD_SET(_sock, &fdRead);
			struct timeval _time;
			_time.tv_sec = 0;
			_time.tv_usec = 0;
			int ret = select(_sock + 1, &fdRead, NULL, NULL, &_time);
			if (ret < 0)
			{
				printf("select 任务结束1\n");
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				if (-1 == RecvData(_sock))
				{
					printf("select 任务结束2\n");
					Close();
					return false;
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
	/*接收数据  处理粘包 拆包*/
	int RecvData(SOCKET clientSock)
	{
		//缓冲区
		char szRecv[1024] = {};

		//5.接受服务端发送来的数据
		//数据存到szRecv中     第三个参数是可接收数据的最大长度
		int nlen = (int)recv(clientSock, szRecv, sizeof(DataHeader), 0);//返回值是接收的长度  MAC修改的地方
		DataHeader* header = (DataHeader*)szRecv;
		if (nlen <= 0)
		{
			printf("客户端<socket=%d>退出,任务结束\n", clientSock);
			return -1;
		}
		recv(clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		OnNetMsg(header);
		return 1;

	}
	//响应网络消息
	void OnNetMsg(DataHeader*header)
	{
		switch (header->cmd)
		{
		case  CMD_LOGINRESULT:
		{
			LoginResult* loginResult = (LoginResult*)header;
			printf("收到服务器消息是 CMD_LOGINRESUL 数据长度:%d\n", loginResult->dataLength);
		}
		break;
		case  CMD_LOGINOUTRESULT:
		{
			LoginOutResult* loginoutResult = (LoginOutResult*)header;
			printf("收到服务器消息是 CMD_LOGINOUTRESULT 数据长度:%d\n", loginoutResult->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			LoginNewUser* loginnewUser = (LoginNewUser*)header;
			printf("收到服务器消息是 CMD_NEW_USER_JOIN 数据长度:%d\n", loginnewUser->dataLength);
		}
		break;
		}
	}
	int SendData(DataHeader*header)
	{
		if (IsRun()&&header!=nullptr)
		{
			return send(_sock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
private:

};
#endif 

