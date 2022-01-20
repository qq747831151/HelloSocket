#include "EasyTcpServer.hpp"
#include<thread>
#include "CellMsgStream.hpp"
class MyServer : public EasyTcpServer
{
public:

	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetJoin(CellClient* pClient)
	{
		EasyTcpServer::OnNetJoin(pClient);
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetLeave(CellClient* pClient)
	{
		EasyTcpServer::OnNetLeave(pClient);
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetMsg(CellServer* pServer, CellClient* pClient, DataHeader* header)
	{
		EasyTcpServer::OnNetMsg(pServer, pClient, header);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			pClient->resetDTHeart();
			//send recv 
			Login* login = (Login*)header;
			//CELLLog::Info("recv <Socket=%d> msgType：CMD_LOGIN, dataLen：%d,userName=%s PassWord=%s\n", cSock, login->dataLength, login->userName, login->PassWord);
			//忽略判断用户密码是否正确的过程
			LoginResult ret;
			if (SOCKET_ERROR == pClient->SendData(&ret))
			{
				//发送缓冲区满了，消息没发出去
				CellLog::Info("<Socket=%d> Send Full\n", pClient->sockfd());
			}
			//netmsg_LoginR* ret = new netmsg_LoginR();
			//pServer->addSendTask(pClient, ret);
		}//接收 消息---处理 发送   生产者 数据缓冲区  消费者 
		break;
		case CMD_LOGINOUT:
		{
			//LoginOut* logout = (LoginOut*)header;
			//CELLLog::Info("recv <Socket=%d> msgType：CMD_LOGOUT, dataLen：%d,userName=%s \n", cSock, logout->dataLength, logout->userName);
			//忽略判断用户密码是否正确的过程
			//netmsg_LogoutR ret;
			//SendData(cSock, &ret);
			pClient->resetDTHeart();
			CellReadStream r(header);
			auto n1 = r.ReadInt8();
			auto n2 = r.ReadInt16();
			auto n3 = r.ReadInt32();
			auto n4 = r.ReadFloat();
			auto n5 = r.ReadDouble();
			uint32_t n = 0;
			r.onlyread(n);
			char name[32] = {};
			auto n6 = r.ReadArray(name, 32);
			char pw[32] = {};
			auto n7 = r.ReadArray(pw, 32);
			int ata[10] = {};
			auto n8 = r.ReadArray(ata, 10);

			///

			CellWriteStream CS(128);
			CS.setNetCmd(CMD_LOGINOUT_RESULT);
			CS.WriteInt8(n1);
			CS.WriteInt16(n2);
			CS.WriteInt32(n3);
			CS.WriteFloat(n4);
			CS.WriteDouble(n5);
			CS.WriteArray(name, n6);
			//const char* str1 = "hahaha";
			CS.WriteArray(pw, n7);
			//int b[] = { 1,2,3,4,5 };
			CS.WriteArray(ata, n8);
			CS.finsh();
			pClient->SendData(CS.Data(), CS.length());
		}
		break;
		case CMD_C2S_HEART:
		{
			pClient->resetDTHeart();
			netmsg_s2c_Heart ret;
			pClient->SendData(&ret);
		}
		default:
		{
			CellLog::Info("recv <socket=%d> undefine msgType,dataLen：%d\n", pClient->sockfd(), header->dataLength);
		}
		break;
		}
	}
private:

};

int main()
{
	CellLog::Instance().setLogPath("serverLog.txt", "w");
	MyServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(64);
	server.Start(4);

	//在主线程中等待用户输入命令
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			server.Close();
			break;
		}
		else {
			CellLog::Info("undefine cmd\n");
		}
	}

	CellLog::Info("exit.\n");
	//#ifdef _WIN32
	//	while (true)
	//		Sleep(10);
	//#endif
	return 0;
}
