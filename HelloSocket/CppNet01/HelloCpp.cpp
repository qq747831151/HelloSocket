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
//	//std::function<void(int)> call = funA;//类似函数指针
//	//call(10);
//
//	std::function<int(int)> call1;
//	int n = 5;
//	//匿名函数 []外部变量捕获列表
//	call1 = [n](int a)->int//()是参数表  ->int是返回类型 没有返回值直接去掉->int 
//	{
//		//函数体
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
///* lambda表达式  拉曼达表达式 匿名函数
//[ caputrue ] ( params ) opt -> ret { body; };
//
//[ 外部变量捕获列表 ] ( 参数表 ) 特殊操作符 -> 返回值类型 { 函数体; };
//
//捕获列表：lambda表达式的捕获列表精细控制了lambda表达式能够访问的外部变量，以及如何访问这些变量。
//
//1) []不捕获任何变量。
//
//2) [&]捕获外部作用域中所有变量，并作为引用在函数体中使用（按引用捕获）。
//
//3) [=]捕获外部作用域中所有变量，并作为副本在函数体中使用(按值捕获)。
//
//4) [=, &foo]按值捕获外部作用域中所有变量，并按引用捕获foo变量。
//
//5) [bar]按值捕获bar变量，同时不捕获其他变量。
//
//6) [this]捕获当前类中的this指针，让lambda表达式拥有和当前类成员函数同样的访问权限。
//如果已经使用了&或者 = ，就默认含有此选项。
//捕获this的目的是可以在lamda中使用当前类的成员函数和成员变量。
//
//////////
//1).capture是捕获列表；
//
//2).params是参数表；(选填)
//
//3).opt是函数选项；可以填mutable,exception,attribute（选填）
//
//mutable说明lambda表达式体内的代码可以修改被捕获的变量，并且可以访问被捕获的对象的non-const方法。
//
//exception说明lambda表达式是否抛出异常以及何种异常。
//
//attribute用来声明属性。
//
//4).ret是返回值类型。(选填)
//
//5).body是函数体。
//*/





//这个是unity 发布到电脑上用的
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

//C++代码调用C语言代码
extern "C" {
	typedef void(*OnNextMsgCallBack)(void* csObj, void* data, int len);
}

class NativeTCPClient : public EasyTcpClient
{
public:
	//响应网络消息
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
