#include "EasyTcpClient.hpp"
bool g_bExit = true;//�߳��˳�
void cmdThread(EasyTcpClient*client)
{
	while (true)
	{
		char szBuf[256];
		scanf("%s", szBuf);
		if (0 == strcmp(szBuf, "exit"))
		{
			printf("�˳�cmdThread\n");
			client->Close();
			break;
		}
		else if (0 == strcmp(szBuf, "login"))
		{    

			Login login;
			strcpy(login.userName, "sfl");
			strcpy(login.passWord, "123");
			client->SendData(&login);
		}
		else if (0 == strcmp(szBuf, "loginout"))
		{
			LoginOut loginOut;
			strcpy(loginOut.userName, "sfl");
			client->SendData(&loginOut);
		}
		else
		{
			printf("��֧������");
		}
	}
}
int main()
{
	EasyTcpClient client1;
	client1.InitSocket();
	client1.Connect("192.168.17.1",4567);
	//�����߳�
	std::thread t1(cmdThread,&client1);
	t1.detach();//�̷߳���
	Login login;
	strcpy(login.passWord, "123");
	strcpy(login.userName,"321");
	while (client1.IsRun())
	{
		client1.OnRun();
		client1.SendData(&login);
	
	}
	client1.Close();


	getchar();
	return 0;
}