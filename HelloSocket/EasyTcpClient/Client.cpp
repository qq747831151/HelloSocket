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
const int Count = 4000;//�ͻ�������
const int tCount = 4;//�߳�����
EasyTcpClient* clients[Count];
//�����߳�
void SendThread(int id)
{
	//�߳�ID =1~4
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
	//���������߳�
	for (int i = 0; i < tCount; i++)
	{
		//�����߳�
		std::thread t1(SendThread,i+1);
		t1.detach();//�̷߳���
	}
	
	//�����߳�
	std::thread t1(cmdThread);
	t1.detach();//�̷߳���


	while (g_bExit)
	{
		Sleep(100);
		
	}
	getchar();
	return 0;
}