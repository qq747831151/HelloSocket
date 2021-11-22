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
const int Count = 10000;//�ͻ�������
const int tCount = 4;//�߳�����
EasyTcpClient* clients[Count];
//�����߳�
void SendThread(int id)
{
	printf("thread<%d> Start\n", id);
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
	printf("thread<%d> Connect <begin=%d   end=%d>\n", id, begin, end);
	std::chrono::milliseconds t(3000);
	std::this_thread::sleep_for(t);
	Login login[10];
	for (int i = 0; i < 10; i++)
	{
		strcpy(login[i].passWord, "123");
		strcpy(login[i].userName, "321");
	}
	const int nlen = sizeof(login);//�������Ӱ�췢�͵İ�  ����Ҫ �����Ͳ��ü������㳤���� Ӱ������
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
	//�����߳�
	std::thread t1(cmdThread);
	t1.detach();//�̷߳���

	//���������߳�
	for (int i = 0; i < tCount; i++)
	{
		//�����߳�
		std::thread t1(SendThread,i+1);
		t1.detach();//�̷߳���
	}
	
	


	while (g_bExit)
	{
		Sleep(100);
		
	}
	getchar();
	return 0;
}