#include "EasyTcpClient.hpp"
#include "CELLTimestamp.hpp"
#include <atomic>
#include <thread>
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
const int Count = 10;//�ͻ�������
const int tCount = 4;//�߳�����
EasyTcpClient* clients[Count];

std::atomic_int sendCount = 0;
std::atomic_int readyCount = 0;
void recvThread(int begin, int end)
{
	CELLTimestamp t;
	while (g_bExit)
	{
		for (int n = begin; n < end; n++)
		{
			
			clients[n]->OnRun();
		}
	}
}
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
		//clients[i]->InitSocket();
		clients[i]->Connect("192.168.17.1", 4567);
	}
	/*������� ��������ʱ*/
	printf("thread<%d> Connect <begin=%d   end=%d>\n", id, begin, end);
	readyCount++;
	while (readyCount<tCount)
	{
		/*�ȴ������߳�׼����һ���� ����*/
		std::chrono::microseconds t(10);//3000����=3��
		std::this_thread::sleep_for(t);
	}
	/*���������߳�*/
	std::thread t1(recvThread, begin, end);
	t1.detach();//�̷߳���

	Login login[10];
	for (int i = 0; i < 10; i++)
	{
		strcpy(login[i].passWord, "123");
		strcpy(login[i].userName, "321");
	}

	const int nlen = sizeof(login);//�������Ӱ�췢�͵İ�  ����Ҫ �����Ͳ��ü������㳤���� Ӱ������
	bool isSend = false;
	while (g_bExit)
	{
		for (int i = begin; i < end; i++)
		{
			
				if (clients[i]->SendData(login, nlen) != -1) {
					sendCount++;
				}
			    /*�ӳ�*/
				std::chrono::milliseconds t(100);//3000����=3��
				std::this_thread::sleep_for(t);
			
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
	
	
	CELLTimestamp t;

	while (g_bExit)
	{
		auto time = t.getElapsedSecond();
		if (time >= 1.0)
		{
			printf("thread<%d>,clients<%d>,time<%lf>,send<%d>\n", tCount, Count, time, (int)(sendCount / time));
			sendCount = 0;
			t.update();
		}
		Sleep(1);
		
	}
	printf("���˳���\n");
	return 0;
}