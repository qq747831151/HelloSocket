//#include <stdio.h>
//#include <functional>
//void funA(int a)
//{
//	printf("funA\n");
//}
//void funB()
//{
//	printf("funB\n");
//}
//
//int main()
//{
//	//void(*p)();
//	//p = funB;
//	//p();
//
//	//std::function<void(int)> call = funA;//���ƺ���ָ��
//	//call(10);
//
//	std::function<int(int)> call1;
//	int n = 5;
//	//�������� []�ⲿ���������б�
//	call1 = [n](int a)->int//()�ǲ�����  ->int�Ƿ������� û�з���ֱֵ��ȥ��->int 
//	{
//		//������
//		printf("funA%d\n", n + a);
//		printf("%d\n", n);
//		return n + a;
//
//	};
//	 n = 7;
//	int b = call1(10);
//	printf("%d\n", b);
//	return 0;
//
//}
///* lambda���ʽ  ��������ʽ ��������
//[ caputrue ] ( params ) opt -> ret { body; };
//
//[ �ⲿ���������б� ] ( ������ ) ��������� -> ����ֵ���� { ������; };
//
//�����б�lambda���ʽ�Ĳ����б�ϸ������lambda���ʽ�ܹ����ʵ��ⲿ�������Լ���η�����Щ������
//
//1) []�������κα�����
//
//2) [&]�����ⲿ�����������б���������Ϊ�����ں�������ʹ�ã������ò��񣩡�
//
//3) [=]�����ⲿ�����������б���������Ϊ�����ں�������ʹ��(��ֵ����)��
//
//4) [=, &foo]��ֵ�����ⲿ�����������б������������ò���foo������
//
//5) [bar]��ֵ����bar������ͬʱ����������������
//
//6) [this]����ǰ���е�thisָ�룬��lambda���ʽӵ�к͵�ǰ���Ա����ͬ���ķ���Ȩ�ޡ�
//����Ѿ�ʹ����&���� = ����Ĭ�Ϻ��д�ѡ�
//����this��Ŀ���ǿ�����lamda��ʹ�õ�ǰ��ĳ�Ա�����ͳ�Ա������
//
//////////
//1).capture�ǲ����б�
//
//2).params�ǲ�����(ѡ��)
//
//3).opt�Ǻ���ѡ�������mutable,exception,attribute��ѡ�
//
//mutable˵��lambda���ʽ���ڵĴ�������޸ı�����ı��������ҿ��Է��ʱ�����Ķ����non-const������
//
//exception˵��lambda���ʽ�Ƿ��׳��쳣�Լ������쳣��
//
//attribute�����������ԡ�
//
//4).ret�Ƿ���ֵ���͡�(ѡ��)
//
//5).body�Ǻ����塣
//*/





//�����unity �������������õ�
#ifndef _Cpp_Net_100_Cpp_
#define  Cpp_Net_100_Cpp_
#include <string>
//#include "EasyTcpClient.hpp"
#include "CellMsgStream.hpp"
#include "EasyTcpClient.hpp"
#pragma  comment(lib, "ws2_32.lib")

#ifdef _WIN32

#define  EXPORT_DLL _declspec(dllexport)

#else

#define EXPORT_DLL

#endif

//C++�������C���Դ���
extern "C" {
	typedef void(*OnNextMsgCallBack)(void* csObj, void* data, int len);
}

class NativeTCPClient : public EasyTcpClient
{
public:
	//��Ӧ������Ϣ
	virtual void OnNetMsg(DataHeader* header)
	{
		if (_callBack)
		{
			_callBack(_csObj, header, header->dataLength);
		}
	}
	void setCallBack(void* csObj, OnNextMsgCallBack cb) {
		_csObj = csObj;
		_callBack = cb;
	}
private:
	void* _csObj = nullptr;
	OnNextMsgCallBack _callBack = nullptr;
};

extern "C" 
{

	//////////////////////////////////////////////////////////////////////////
	EXPORT_DLL int  add(int a, int b) {
		return a + b;
	}

	typedef void(*Callback1)(const char* str);

	EXPORT_DLL void  TestCall(const char* str1, Callback1 cb) {
		std::string s = "Hello C#";
		s += str1;
		cb(s.c_str());
	}

	////////////////////////////////////CellClient

	EXPORT_DLL void* CellClient_Create(void* csObj, OnNextMsgCallBack cb, int sendSize, int recvSize)
	{
		NativeTCPClient* pClient = new NativeTCPClient();
		pClient->setCallBack(csObj, cb);
		pClient->InitSocket(sendSize,recvSize);
		return pClient;
	}
	EXPORT_DLL bool  CellClient_Connect(NativeTCPClient* pClient, const char* ip, short port)
	{
		if (pClient && ip)
		{
			return pClient->Connect(ip, port) != SOCKET_ERROR;
		}
		return false;

	}
	EXPORT_DLL bool  CellClient_OnRun(NativeTCPClient* pClient)
	{
		if (pClient)
		{
			return pClient->OnRun();
		}
		return false;

	}
	EXPORT_DLL void  CellClient_OnClose(NativeTCPClient* pClient)
	{
		if (pClient) {
			pClient->Close();
			delete pClient;
		}
	}
	EXPORT_DLL int  CellClient_SendData(NativeTCPClient* pClient, const char* data, int len)
	{
		if (pClient)
			return pClient->SendData(data, len);
		return 0;

	}
	EXPORT_DLL int  CellClient_SendWriteStream(NativeTCPClient* pClient, CellWriteStream* wStream)
	{
		if (pClient && wStream) {
			wStream->finsh();
			return pClient->SendData(wStream->Data(), wStream->length());
		}
		return 0;

	}

	///////////////////////////////////CellStream
/////////////CellWriteStream

	EXPORT_DLL void* CellWriteStream_Create(int nSize)
	{
		CellWriteStream* wStream = new CellWriteStream(nSize);
		return wStream;
	}

	EXPORT_DLL bool CellWriteStream_WriteInt8(CellWriteStream* wStream, int8_t n)
	{
		if (wStream)
			return wStream->Write(n);
		return false;

	}
	EXPORT_DLL bool CellWriteStream_WriteInt16(CellWriteStream* wStream, int16_t n)
	{
		if (wStream)
			return wStream->Write(n);
		return false;

	}
	EXPORT_DLL bool CellWriteStream_WriteInt32(CellWriteStream* wStream, int32_t n)
	{
		if (wStream)
			return wStream->Write(n);
		return false;

	}
	EXPORT_DLL bool CellWriteStream_WriteInt64(CellWriteStream* wStream, int64_t n)
	{
		if (wStream)
			return wStream->Write(n);
		return false;

	}


	EXPORT_DLL bool CellWriteStream_WriteUInt8(CellWriteStream* wStream, uint8_t n)
	{
		if (wStream)
			return wStream->Write(n);
		return false;

	}
	EXPORT_DLL bool CellWriteStream_WriteUInt16(CellWriteStream* wStream, uint16_t n)
	{
		if (wStream)
			return wStream->Write(n);
		return false;

	}
	EXPORT_DLL bool CellWriteStream_WriteUInt32(CellWriteStream* wStream, uint32_t n)
	{
		if (wStream)
			return wStream->Write(n);
		return false;

	}
	EXPORT_DLL bool CellWriteStream_WriteUInt64(CellWriteStream* wStream, uint64_t n)
	{
		if (wStream)
			return wStream->Write(n);
		return false;

	}

	EXPORT_DLL bool CellWriteStream_WriteFloat(CellWriteStream* wStream, float n)
	{
		if (wStream)
			return wStream->Write(n);
		return false;

	}
	EXPORT_DLL bool CellWriteStream_WriteDouble(CellWriteStream* wStream, double n)
	{
		if (wStream)
			return wStream->Write(n);
		return false;

	}

	EXPORT_DLL bool CellWriteStream_WriteString(CellWriteStream* wStream, char* n)
	{
		if (wStream)
			return wStream->WriteString(n);

		return false;

	}
	EXPORT_DLL void CellWriteStream_Release(CellWriteStream* wStream)
	{
		if (wStream)
		{
			delete wStream;
		}

	}

	/////////////////CellReadStream
	EXPORT_DLL void* CellReadStream_Create(char* data, int len)
	{
		CellReadStream* r = new CellReadStream(data, len);
		return r;
	}

	EXPORT_DLL int8_t CellReadStream_ReadInt8(CellReadStream* rStream)
	{
		if (rStream)
		{
			return rStream->ReadInt8();
		}
		return 0;

	}
	EXPORT_DLL int16_t CellReadStream_ReadInt16(CellReadStream* rStream)
	{
		if (rStream)
		{
			return rStream->ReadInt16();
		}
		return 0;

	}
	EXPORT_DLL int32_t CellReadStream_ReadInt32(CellReadStream* rStream)
	{
		if (rStream)
		{
			return rStream->ReadInt32();
		}
		return 0;
	}
	EXPORT_DLL int64_t CellReadStream_ReadInt64(CellReadStream* rStream)
	{
		if (rStream)
		{
			return rStream->ReadInt64();
		}
		return 0;
	}


	EXPORT_DLL int8_t CellReadStream_ReadUInt8(CellReadStream* rStream)
	{
		if (rStream)
		{
			return rStream->ReadUInt8();
		}
		return 0;

	}
	EXPORT_DLL int16_t CellReadStream_ReadUInt16(CellReadStream* rStream)
	{
		if (rStream)
		{
			return rStream->ReadUInt16();
		}
		return 0;

	}
	EXPORT_DLL int32_t CellReadStream_ReadUInt32(CellReadStream* rStream)
	{
		if (rStream)
		{
			return rStream->ReadUInt32();
		}
		return 0;
	}
	EXPORT_DLL int64_t CellReadStream_ReadUInt64(CellReadStream* rStream)
	{
		if (rStream)
		{
			return rStream->ReadUInt64();
		}
		return 0;
	}

	EXPORT_DLL float CellReadStream_ReadFloat(CellReadStream* rStream)
	{
		if (rStream)
		{
			return rStream->ReadFloat();
		}
		return 0.0f;
	}
	EXPORT_DLL double CellReadStream_ReadDouble(CellReadStream* rStream)
	{
		if (rStream)
		{
			return rStream->ReadDouble();
		}
		return 0.0;
	}

	EXPORT_DLL bool CellReadStream_ReadString(CellReadStream* rStream, char* pBuffer, int len)
	{
		if (rStream && pBuffer)
		{
			return rStream->ReadArray(pBuffer, len);
		}
		return false;
	}
	EXPORT_DLL void CellReadStream_Release(CellReadStream* rStream)
	{
		if (rStream)

			delete rStream;


	}
	EXPORT_DLL uint32_t CellReadStream_OnlyReadUInt32(CellReadStream* rStream)
	{
		uint32_t len = 0;
		if (rStream)
		{
			rStream->onlyread(len);
		}
		return len;
	}
}
#endif
