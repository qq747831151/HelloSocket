#define WIN32_LEAN_AND_MEAN

#define _WINSOCK_DEPRECATED_NO_WARNINGS //影响inet_addr
#include <windows.h>
#include <WINSock2.h>
#include <stdio.h>
#include <thread>

/*为了可以在其他平台也可以使用 右键项目属性 选择链接器 附加依赖项 将ws2_32.lib 添加进去就行 这样就不需要 下面这些 */
#pragma  comment(lib,"ws2_32.lib")
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
struct Login:DataHeader
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
struct LoginResult:DataHeader
{
	LoginResult()
	{
		cmd = CMD_LOGINRESULT;
		dataLength = sizeof(LoginResult);
	}
	int result=1;
};
//登出
struct LoginOut:DataHeader
{
	LoginOut()
	{
		cmd = CMD_LOGINOUT;
		dataLength = sizeof(LoginOut);

	}
	char userName[32];

};
//登录结果
struct LoginOutResult:DataHeader
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
int Processor(SOCKET clientSock)
{
	//缓冲区
	char szRecv[1024] = {};

	//5.接受服务端发送来的数据
	//数据存到szRecv中     第三个参数是可接收数据的最大长度
	int nlen = recv(clientSock, szRecv, sizeof(DataHeader), 0);//返回值是接收的长度
	DataHeader* header = (DataHeader*)szRecv;
	if (nlen <= 0)
	{
		printf("客户端<socket=%d>退出,任务结束\n", clientSock);
		return -1;
	}
	switch (header->cmd)
	{
	case  CMD_LOGINRESULT:
	{
		recv(clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginResult* loginResult = (LoginResult*)szRecv;
		printf("收到服务器消息是 CMD_LOGINRESUL 数据长度:%d\n", loginResult->dataLength);
	}
	break;
	case  CMD_LOGINOUTRESULT:
	{
		recv(clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginOutResult* loginoutResult = (LoginOutResult*)szRecv;
		printf("收到服务器消息是 CMD_LOGINOUTRESULT 数据长度:%d\n", loginoutResult->dataLength);
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		recv(clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginNewUser* loginnewUser = (LoginNewUser*)szRecv;
		printf("收到服务器消息是 CMD_NEW_USER_JOIN 数据长度:%d\n", loginnewUser->dataLength);
	}
	break;
	}
}
bool g_bExit = true;//线程退出
void cmdThread(SOCKET sock)
{
	while (true)
	{
		char szBuf[256];
		scanf("%s", szBuf);
		if (0 == strcmp(szBuf, "exit"))
		{
			printf("退出cmdThread\n");
			g_bExit = false;
			break;
		}
		else if (0 == strcmp(szBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "sfl");
			strcpy(login.passWord, "123");
			send(sock, (char*)&login, sizeof(login), 0);
		}
		else if (0 == strcmp(szBuf, "loginout"))
		{
			LoginOut loginOut;
			strcpy(loginOut.userName, "sfl");
			send(sock, (char*)&loginOut, sizeof(loginOut), 0);
		}
		else
		{
			printf("不支持命令");
		}
	}
}
int main()
{
	/*启动socket网络环境 2.x环境*/
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	/*编写*/
	//--用Socket API建立简易的TCP客户端
	//1.建立一个socket套接字 
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock==INVALID_SOCKET)
	{
		printf("ERROR,建立套接字失败\n");
	}
	else
	{
		printf("TRUE,建立套接字成功\n");
	}
	//2.连接服务器 connect
	sockaddr_in _sin;
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret=connect(sock, (struct sockaddr*)&_sin, sizeof(_sin));
	if (ret == INVALID_SOCKET)
	{
		printf("ERROR,连接服务器connect失败........\n");
	}
	else
	{
		printf("TRUE,建连接服务器connect成功.......\n");
	}
	//启动线程
	std::thread t1(cmdThread,sock);
	t1.detach();//线程分离
	while (g_bExit)
	{

		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(sock, &fdRead);
		struct timeval _time;
		_time.tv_sec = 0;
		_time.tv_usec = 0;
		int ret = select(sock + 1, &fdRead, NULL, NULL, &_time);
		if (ret<0)
		{
			printf("select 任务结束1\n");
			break;;
		}
		if (FD_ISSET(sock,&fdRead))
		{
			FD_CLR(sock, &fdRead);
			if (-1==Processor(sock))
			{
				printf("select 任务结束2\n");
					break;
			}
		}
	
	}
	//7.关闭套接字
	closesocket(sock);
	WSACleanup();
	getchar();
	return 0;
}