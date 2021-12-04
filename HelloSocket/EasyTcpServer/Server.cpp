#pragma  once
#include "EasyTcpServer.hpp"
#include "Alloc.h"
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
class MyServer :public EasyTcpServer
{
public:
	//客户端离开事件 CellServer 4 多个线程触发不安全 如果只开启1个cellServer 就是安全的
	virtual void OnNetLeave(ClientScoket*pClient)
	{
		EasyTcpServer::OnNetLeave(pClient);
	}
	/*客户端消息事件*/
	virtual void OnNetMsg(CellServer*pCellServer, DataHeader* header, ClientScoket *pClient)
	{
		EasyTcpServer::OnNetMsg(pCellServer,header, pClient);
		switch (header->cmd)
		{
		case CMD_LOGIN: {
			Login* login = (Login*)header;
			//printf("收到命令:%d 数据长度：%d username:%s password:%s\n", login->cmd, login->dataLength, login->userName, login->passWord);
			/*忽略 判断用户名密码是否正确*/
			LoginResult* loginresult=new LoginResult();
			//pClient->SendData(&loginresult);
			pCellServer->addSendTask(pClient, loginresult);
		}
			break;
		case CMD_LOGINOUT: {
			LoginOut* loginout = (LoginOut*)header;
			//printf("收到命令:%d 数据长度：%d username:%s password:%s\n", loginout->cmd, loginout->dataLength, loginout->userName);
			/*忽略 判断用户名密码是否正确*/
		//	LoginOutResult loginOutresult;
			//pClient->SendData(&loginOutresult);
		}
					  break;
		default:
			printf("<socket=%d>收到未定义消息 数据长度：%d \n", pClient->Getsockfd(), header->dataLength);
			//DataHeader dp;
			//SendData(&dp, _clientSock);
			break;
		}
	}
	/*客户端加入事件*/
	virtual void OnNetJoin(ClientScoket* pClient)
	{
		EasyTcpServer::OnNetJoin(pClient);
	}
	/*virtual void OnNetRecv(ClientScoket* pClient)
	{
		_recvCount++;
	}*/
};
int main()
{

	MyServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
	server.Start(4);

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
