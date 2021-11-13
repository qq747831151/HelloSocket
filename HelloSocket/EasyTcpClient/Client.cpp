#include "EasyTcpClient.hpp"
bool g_bExit = true;//�߳��˳�
void cmdThread()
{
	while (true)
	{
		char szBuf[256];
		scanf("%s", szBuf);
		if (0 == strcmp(szBuf, "exit"))
		{
			printf("�˳�cmdThread\n");
			g_bExit = false;
			break;
		}
		else
		{
			printf("��֧������");
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
	
	//�����߳�
	std::thread t1(cmdThread);
	t1.detach();//�̷߳���

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