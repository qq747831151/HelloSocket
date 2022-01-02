﻿
#include "EasyTcpServer.hpp"
#include "Alloc.h"
#include <thread>
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
	/*客户端加入事件*/
	virtual void OnNetJoin(CellClient* pClient)
	{
		EasyTcpServer::OnNetJoin(pClient);
	}

	//客户端离开事件 CellServer 4 多个线程触发不安全 如果只开启1个cellServer 就是安全的
	virtual void OnNetLeave(CellClient*pClient)
	{
		EasyTcpServer::OnNetLeave(pClient);
	}
	/*客户端消息事件*/
	virtual void OnNetMsg(CellServer*pCellServer, DataHeader* header, CellClient *pClient)
	{
		EasyTcpServer::OnNetMsg(pCellServer,header, pClient);
		switch (header->cmd)
		{
		case CMD_LOGIN: {

			pClient->resetDtHeart();

			Login* login = (Login*)header;
			//printf("收到命令:%d 数据长度：%d username:%s password:%s\n", login->cmd, login->dataLength, login->userName, login->passWord);
			/*忽略 判断用户名密码是否正确*/
			LoginResult loginresult;
			pClient->SendData(&loginresult);
			//pCellServer->addSendTask(pClient, loginresult);
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
		case  CMD_HEART_C2S:

		{

			/*假如客户端有发送消息过来,就认为客户端有心跳了 重置计时为0*/
			pClient->resetDtHeart();
			netmsg_s2c_Heart ret;
			pClient->SendData(&ret);
		}
		break;
		default:
			printf("<socket=%d>收到未定义消息 数据长度：%d \n", pClient->Getsockfd(), header->dataLength);
			//DataHeader dp;
			//SendData(&dp, _clientSock);
			break;
		}
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
