#include "EasyTcpServer.hpp"
#include <thread>
#include <stdio.h>
bool g_bExit = true;//线程退出
void cmdThread()
{
	while (true)
	{
		char szBuf[256];
		scanf("%s", szBuf);
		if (0 == strcmp(szBuf, "exit"))
		{
			g_bExit = false;
			printf("退出cmdThread\n");
			break;
		}
		else
		{
			printf("不支持命令\n");
		}

	}
}
int main()
{

	EasyTcpServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
	std::thread t1(cmdThread);
	t1.detach();
	while (g_bExit)
	{
		server.OnRun();
	}
	server.Close();
	getchar();
	return 0;

}
