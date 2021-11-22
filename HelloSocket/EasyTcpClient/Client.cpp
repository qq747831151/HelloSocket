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
const int Count = 10000;//客户端数量
const int tCount = 4;//线程数量
EasyTcpClient* clients[Count];
//发送线程
void SendThread(int id)
{
	printf("thread<%d> Start\n", id);
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
	printf("thread<%d> Connect <begin=%d   end=%d>\n", id, begin, end);
	std::chrono::milliseconds t(3000);
	std::this_thread::sleep_for(t);
	Login login[10];
	for (int i = 0; i < 10; i++)
	{
		strcpy(login[i].passWord, "123");
		strcpy(login[i].userName, "321");
	}
	const int nlen = sizeof(login);//这个严重影响发送的包  很重要 这样就不用继续计算长度了 影响性能
	while (g_bExit)
	{
		for (int i = begin; i < end; i++)
		{

			clients[i]->SendData(login,nlen);
			//clients[i]->OnRun();
		}

	}
	for (int i = begin; i < end; i++)
	{
		clients[i]->Close();
		delete clients[i];
	}
	printf("thread<%d>exit\n", id);
}
int main()
{
	//启动线程
	std::thread t1(cmdThread);
	t1.detach();//线程分离

	//启动发送线程
	for (int i = 0; i < tCount; i++)
	{
		//启动线程
		std::thread t1(SendThread,i+1);
		t1.detach();//线程分离
	}
	
	


	while (g_bExit)
	{
		Sleep(100);
		
	}
	getchar();
	return 0;
}