#include "EasyTcpClient.hpp"
bool g_bExit = true;//线程退出
void cmdThread(EasyTcpClient*client)
{
	while (true)
	{
		char szBuf[256];
		scanf("%s", szBuf);
		if (0 == strcmp(szBuf, "exit"))
		{
			printf("退出cmdThread\n");
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
			printf("不支持命令");
		}
	}
}
int main()
{
	EasyTcpClient client1;
	client1.InitSocket();
	client1.Connect("192.168.17.1",4567);
	//启动线程
	std::thread t1(cmdThread,&client1);
	t1.detach();//线程分离
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