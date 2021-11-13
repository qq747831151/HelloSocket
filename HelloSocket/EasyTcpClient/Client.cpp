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
const int Count = 4000;//客户端数量
const int tCount = 4;//线程数量
EasyTcpClient* clients[Count];
//发送线程
void SendThread(int id)
{
	//线程ID =1~4
	int c = (Count / tCount);
	int begin = (id - 1) * c;
	int end = id * c;
	for (int i = begin; i < end; i++)
	{
		clients[i] = new EasyTcpClient();
	}
	for (int i = begin; i < end; i++)
	{
		clients[i]->InitSocket();
		clients[i]->Connect("192.168.17.1", 4567);
	}
	Login login;
	strcpy(login.passWord, "123");
	strcpy(login.userName, "321");
	while (g_bExit)
	{
		for (int i = begin; i < end; i++)
		{

			clients[i]->SendData(&login);
			//clients[i]->OnRun();
		}

	}
	for (int i = begin; i < end; i++)
	{
		clients[i]->Close();
	}
}
int main()
{
	//启动发送线程
	for (int i = 0; i < tCount; i++)
	{
		//启动线程
		std::thread t1(SendThread,i+1);
		t1.detach();//线程分离
	}
	
	//启动线程
	std::thread t1(cmdThread);
	t1.detach();//线程分离


	while (g_bExit)
	{
		Sleep(100);
		
	}
	getchar();
	return 0;
}