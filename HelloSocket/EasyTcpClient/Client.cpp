#include "EasyTcpClient.hpp"
bool g_bExit = true;//线程退出
void cmdThread()
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
		else
		{
			printf("不支持命令");
		}
	}
}
int main()
{
	const int Count = FD_SETSIZE - 1;
	EasyTcpClient* clients[Count];
	for (int n = 0; n < Count; n++)
	{
		clients[n] = new EasyTcpClient[n];
	}
	for (int i = 0; i < Count; i++)
	{
		clients[i]->InitSocket();
		clients[i]->Connect("192.168.17.1", 4567);
	}
	
	//启动线程
	std::thread t1(cmdThread);
	t1.detach();//线程分离

	Login login;
	strcpy(login.passWord, "123");
	strcpy(login.userName,"321");

	while (g_bExit)
	{
		for (int i = 0; i < Count; i++)
		{
			clients[i]->SendData(&login);
			clients[i]->OnRun();
		}
	}
	for (int i = 0; i < Count; i++)
	{
		clients[i]->Close();
	}
	
	getchar();
	return 0;
}