#define WIN32_LEAN_AND_MEAN

#define _WINSOCK_DEPRECATED_NO_WARNINGS //影响inet_addr
#include <windows.h>
#include <WINSock2.h>
#include <stdio.h>

/*为了可以在其他平台也可以使用 右键项目属性 选择链接器 附加依赖项 将ws2_32.lib 添加进去就行 这样就不需要 下面这些 */
#pragma  comment(lib,"ws2_32.lib")

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
		printf("ERROR,建立套接字失败");
	}
	else
	{
		printf("TRUE,建立套接字成功");
	}
	//2.连接服务器 connect
	sockaddr_in _sin;
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret=connect(sock, (struct sockaddr*)&_sin, sizeof(_sin));
	if (ret == INVALID_SOCKET)
	{
		printf("ERROR,连接服务器connect失败........");
	}
	else
	{
		printf("TRUE,建连接服务器connect成功.......");
	}
	while (true)
	{
		//3.输入请求命令
		char recvBuf[128] = "";
		scanf("%s", recvBuf);
		//4.处理请求命令
		if (strcmp(recvBuf,"exit")==0)
		{
			printf("收到exit命令,任务结束");
			break;
		}
		else
		{
			//5.向服务器发送请求
			send(sock, recvBuf, strlen(recvBuf) + 1, 0);
		}
		//6.接收服务器信息
		char msgBuf[128] = "";
		int nlen = recv(sock, msgBuf, sizeof(msgBuf), 0);
		printf("%d\n", nlen);
		if (nlen>0)
		{
			printf("接收到的数据为%s\n", msgBuf);
		}
	}
	//7.关闭套接字
	closesocket(sock);
	WSACleanup();
	getchar();
	return 0;
}