//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
//#include <WinSock2.h>
///*为了可以在其他平台也可以使用 右键项目属性 选择链接器 附加依赖项 将ws2_32.lib 添加进去就行 这样就不需要 下面这些 */
////#pragma comment(lib,"ws2_32.lib")
//int  main()
//{
//	/*启动socket网络环境 2.x环境*/
//	WORD ver = MAKEWORD(2, 2);
//	WSADATA dat;
//	WSAStartup(ver, &dat);
//	/*编写*/
//	WSACleanup();
//	return 0;
//}


#include"EasyTcpClient.hpp"
#include"CELLStream.hpp"
#include"CELLMsgStream.hpp"

class MyClient : public EasyTcpClient
{
public:
	//响应网络消息
	virtual void OnNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGINOUT_RESULT:
		{
			CellReadStream r(header);
			//读取消息长度
			r.ReadInt16();
			//读取消息命令
			r.getNetCmd();
			auto n1 = r.ReadInt8();
			auto n2 = r.ReadInt16();
			auto n3 = r.ReadInt32();
			auto n4 = r.ReadFloat();
			auto n5 = r.ReadDouble();
			char name[32] = {};
			auto n6 = r.ReadArray(name, 32);
			char pw[32] = {};
			auto n7 = r.ReadArray(pw, 32);
			int ata[10] = {};
			auto n8 = r.ReadArray(ata, 10);

		}
		break;
		case CMD_ERROR:
		{
			//CELLLog::Info("<socket=%d> recv msgType：CMD_ERROR\n", (int)_pClient->sockfd());
		}
		break;
		default:
		{
			//CELLLog::Info("error, <socket=%d> recv undefine msgType\n", (int)_pClient->sockfd());
		}
		}
	}
private:

};

int main()
{

	CellWriteStream s(128);
	s.setNetCmd(CMD_LOGINOUT);
	s.WriteInt8(1);
	s.WriteInt16(2);
	s.WriteInt32(3);
	s.WriteFloat(4.5f);
	s.WriteDouble(6.7);
	s.WriteString("client");

	char atr1[] = "hahaha";
	s.WriteArray(atr1, strlen(atr1));
	int b[] = { 1,2,3,4,5 };
	s.WriteArray(b, 5);
	s.finsh();
	MyClient client;
	client.Connect("192.168.17.1", 4567);


	while (client.OnRun())
	{
		client.SendData(s.Data(), s.length());
		CellThread::Sleep1(10);
	}
	return 0;
}