
#include "EasyTcpServer.hpp"
#include "Alloc.h"
#include <thread>

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

			/*假如客户端有发送消息过来 就认为客户端有心跳了 重置计时为0*/
			pClient->resetDtHeart();

			Login* login = (Login*)header;
			//printf("收到命令:%d 数据长度：%d username:%s password:%s\n", login->cmd, login->dataLength, login->userName, login->passWord);
			/*忽略 判断用户名密码是否正确*/
			LoginResult loginresult;
			
			if (pClient->SendData(&loginresult) == -1) 
			{
				//消息发送缓冲区满了,消息没发出去
				printf("Socket=%d SendF ull \n", pClient->Getsockfd());
			}
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
	server.Listen(64);
	server.Start(4);


	while (true)
	{
		char szBuf[256];
		scanf("%s", szBuf);
		if (0 == strcmp(szBuf, "exit"))
		{
			printf("退出cmdThread\n");
			break;
		}
		else
		{
			printf("不支持命令\n");
		}
	}
	printf("exit.\n");
#ifdef _WIN32
	while (true)
		Sleep(10);
#endif
	return 0;

}
