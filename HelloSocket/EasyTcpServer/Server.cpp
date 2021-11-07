#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN//不影响 windows.h 和 WinSock2.h 前后顺序 
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

#define _WINSOCK_DEPRECATED_NO_WARNINGS //这个用于 inet_ntoa   可以在右击项目属性 C/C++ 预处理里面 预处理定义添加


#include <stdio.h>
#include <stdlib.h>
#include <vector>


enum CMD
{
	CMD_LOGIN,
	CMD_LOGINRESULT,
	CMD_LOGINOUT,
	CMD_LOGINOUTRESULT,
	CMD_ERROR,
	CMD_NEW_USER_JOIN,

};
//消息头
struct DataHeader
{
	short cmd;//命令
	short dataLength;//数据长度
};
//登录
struct Login :DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];
};
//登录结果
struct LoginResult :DataHeader
{
	LoginResult()
	{
		cmd = CMD_LOGINRESULT;
		dataLength = sizeof(LoginResult);
	}
	int result=1;
};
//登出
struct LoginOut :DataHeader
{
	LoginOut()
	{
		cmd = CMD_LOGINOUT;
		dataLength = sizeof(LoginOut);

	}
	char userName[32];

};
//登出结果
struct LoginOutResult :DataHeader
{
	LoginOutResult()
	{
		cmd = CMD_LOGINOUTRESULT;
		dataLength = sizeof(LoginOutResult);
	}
	int result=1;
};
//新用户加入
struct LoginNewUser :DataHeader
{
	LoginNewUser()
	{
		cmd = CMD_NEW_USER_JOIN;
		dataLength = sizeof(LoginNewUser);
		sock = 0;
	}
	int sock;
};
std::vector<SOCKET> g_clients;
int Processor(SOCKET clientSock)
{
	//缓冲区
	char szRecv[1024] = {};
	//5.接受客户端请求数据
	//数据存到szRecv中     第三个参数是可接收数据的最大长度
	int nlen = recv(clientSock, szRecv, sizeof(DataHeader), 0);//返回值是接收的长度  revcz在mac返回值是long 建议强转int
	DataHeader* header = (DataHeader*)szRecv;
	if (nlen <= 0)
	{
		printf("客户端<socket=%d>退出,任务结束",clientSock);
		return -1;
	}
	printf("收到命令:%d 数据长度%d\n", header->cmd, header->dataLength);
	//6.处理请求
	switch (header->cmd)
	{
	case CMD_LOGIN:
	{
		recv(clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login* login = (Login*)szRecv;
		printf("收到的命令:%d 数据长度%d userName:%s password:%s\n", login->cmd, login->dataLength, login->userName, login->passWord);
		//忽略 判断用户名密码是否正确
		LoginResult loginresult;
		send(clientSock, (char*)&loginresult, sizeof(LoginResult), 0);
	}
	break;
	case  CMD_LOGINOUT:
	{
		recv(clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginOut* loginout = (LoginOut*)szRecv;
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
int main()
{
#ifdef _WIN32
	/*启动socket网络环境 2.x环境*/
	WORD ver = MAKEWORD(2, 2);//版本号
	WSADATA dat;
	WSAStartup(ver, &dat);//动态库需要写上那个lib
#endif
	

	//1.建立Socket API 建立简易TC服务端
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//2.绑定 用于接受客户端连接的网络端口
	sockaddr_in _sin = {  };
	_sin.sin_family = AF_INET;//ipv4
	_sin.sin_port = htons(4567);//套字节序应该是网络字节序
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = ADDR_ANY;//获取IP得操作交给内核
//_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");如果项目只是内网使用的话可以使用127
#else
	//inet_pton(AF_INET, "192.168.17.1", &_sin.sin_addr.s_addr);
	_sin.sin_addr.s_addr = ADDR_ANY;
#endif

	int ret = bind(sock, (struct sockaddr*)&_sin, sizeof(_sin));
	if (ret == SOCKET_ERROR)
	{
		printf("ERROR,绑定用于接受客户端连接的网络端口失败...\n");
	}
	else
	{
		printf("TURE,绑定用于接受客户端连接的网络端口成功.....\n");
	}
	//3.监听网络端口
	ret = listen(sock, 5);
	if (ret == SOCKET_ERROR)
	{
		printf("ERROR,监听网络端口失败...\n");
	}
	else
	{
		printf("TURE,监听网络端口成功.....\n");
	}
//	char cmBuf[128] = {};
	while (1)
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
		FD_SET(sock, &fdRead);
		FD_SET(sock, &fdWrite);
		FD_SET(sock, &fdExp);//将参数文件描述符fd对应的标志位,设置为1
		SOCKET maxSock = sock;
		for (int i = g_clients.size()-1; i >=0; i--)
		{
			FD_SET(g_clients[i], &fdRead);//可以放只读 另外两个也可以放
			if (g_clients[i]>maxSock)
			{
				maxSock = g_clients[i];
			}
		}
		struct timeval _time;
		_time.tv_sec = 0;
		_time.tv_usec = 0;
		//nfds 是一个整数值 是指fd_set集合中所有的描述符socket 的范围,而不是数量,
		//既是所有文件描述符最大值+1在windows无所谓 在linux是这样的
		int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &_time);
		if (ret<0)
		{
			printf("客户端已退出,任务结束\n");
			break;
		}

		// 判断fd对应的标志位到底是0还是1, 返回值: fd对应的标志位的值, 0, 返回0, 1->返回1
		//有新连接
		//判断描述符(socket)是否在集合中
		if (FD_ISSET(sock,&fdRead))
		{
			FD_CLR(sock, &fdRead);// 将参数文件描述符fd对应的标志位, 设置为0
			//4.等待接收客户端连接
			sockaddr_in addCli = {};
			int nlen = sizeof(sockaddr_in);
			SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
			_cSock = accept(sock, (sockaddr*)&addCli, &nlen);
#else
			_cSock = accept(sock, (sockaddr*)&addCli, (socklen_t*)&nlen);
#endif
		
			if (_cSock==INVALID_SOCKET)
			{
				printf("等待连接客户端失败......\n");
			}
			else
			{
				//如果有新客户端加入 就向其他现有的客户端发送
				for (int i = g_clients.size() - 1; i >= 0; i--)
				{
					LoginNewUser newuser;
					send(g_clients[i], (const char*)&newuser, sizeof(LoginNewUser), 0);
				}
				printf("新客户端加入：socket=%d IP=%s\n", (int)_cSock, inet_ntoa(addCli.sin_addr));
				g_clients.push_back(_cSock);
			}
			
		}
		////通信,有客户端发送数据过来  在windows中 fd_read.fd_count 认为是保留发生事件的socket的数量  这个只能用在windows中
		//for (int i = 0; i < fdRead.fd_count; i++)
		//{
		//	
		//	if (-1==Processor(fdRead.fd_array[i]))
		//	{
		//		auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[i]);
		//		if (iter!=g_clients.end())
		//		{
		//			g_clients.erase(iter);
		//		}
		//	}
		//}


		//通信,有客户端发送数据过来
		for (int i = g_clients.size()-1; i >= 0; i--)
		{
			if (FD_ISSET(g_clients[i],&fdRead))
			{
				if (-1==Processor(g_clients[i]))
				{
					std::vector<SOCKET>::iterator iter = g_clients.begin() + i;;//i是删除的迭代器
					if (iter != g_clients.end())
					{
						g_clients.erase(iter);//删除
					}
				}
			}
		}
	
	}
#ifdef _WIN32
	for (int i = g_clients.size() - 1; i >= 0; i--)
	{
		closesocket(g_clients[i]);
	}
	//8 关闭套接字
	closesocket(sock);
	WSACleanup();
#else
	for (int i = g_clients.size() - 1; i >= 0; i--)
	{
		close(g_clients[i]);
	}
	//8 关闭套接字
	close(sock);
#endif

	getchar();
	return 0;

}
