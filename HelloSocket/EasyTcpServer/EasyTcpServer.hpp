
#ifndef _EasyTcpServer_HPP
#define  _EasyTcpServer_HPP

#include "Cell.hpp"
#include "CellClient.hpp"
#include "CellServer.hpp"
#include "INetEvent.hpp"
#include "CEllObjectPool.hpp"
#include "CellLog.hpp"
#include "CellNetWork.hpp"
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
class EasyTcpServer:public INetEvent
{
private:
	SOCKET _sock;
	/*消息处理对象,内部会创建线程*/
	std::vector<CellServer*>_cellServer;
	/*每秒消息计时*/
	CELLTimestamp _time;

	CellThread _thread;
protected:
	std::atomic_int _recvCount;//收到消息计数
	std::atomic_int  _ClientCount;//客户端计数
	std::atomic_int  _MsgCount;//SOCKET recv函数计数
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;//无效地址
		_recvCount = 0;
		_ClientCount = 0;
		_MsgCount = 0;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}
	//初始化
	SOCKET InitSocket()
	{
		CellNetWork::Init();
		if (_sock!=INVALID_SOCKET)
		{
			CellLog::Info("<Socket=%d>关闭之前的旧链接\n", _sock);
			Close();
		}
	//1.建立Socket API 建立简易TC服务端
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (_sock == INVALID_SOCKET)
		{
			CellLog::Info("ERROR,<socket=%d>建立socket失败...\n", _sock);
		}
		else
		{
			CellLog::Info("TURE,<socket=%d>建立socket成功.....\n", _sock);
		}
		return _sock;
	}
	int Bind(const char *ip,unsigned short port)
	{

		//2.绑定 用于接受客户端连接的网络端口
		sockaddr_in _sin = {  };
		_sin.sin_family = AF_INET;//ipv4
		_sin.sin_port = htons(port);//套字节序应该是网络字节序
#ifdef _WIN32
		if (ip)
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);;
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = ADDR_ANY;//获取IP得操作交给内核
		}
	
	//_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");如果项目只是内网使用的话可以使用127
#else
		if (ip)
		{
			inet_pton(AF_INET, ip, &_sin.sin_addr.s_addr);
		}
		else
		{
			_sin.sin_addr.s_addr = ADDR_ANY;
		}
	//inet_pton(AF_INET, "192.168.17.1", &_sin.sin_addr.s_addr);
		
#endif

		int ret = bind(_sock, (struct sockaddr*)&_sin, sizeof(_sin));
		if (ret == SOCKET_ERROR)
		{
			CellLog::Info("ERROR,绑定用于接受客户端连接的网络端口失败...\n");
		}
		else
		{
			CellLog::Info("TURE,绑定用于接受客户端连接的网络端口成功.....\n");
		}
		return ret;
	}
	//监听端口号
	int Listen(int n)
	{
		//3.监听网络端口
		int ret = listen(_sock, n);
		if (ret == SOCKET_ERROR)
		{
			CellLog::Info("ERROR,监听网络端口失败...\n");
		}
		else
		{
			CellLog::Info("TURE,监听网络端口成功.....\n");
		}
		return ret;
	}
	//接收客户端连接
	SOCKET Accept()
	{
		//4.等待接收客户端连接
		sockaddr_in addCli = {};
		int nlen = sizeof(sockaddr_in);
		SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32
		_cSock = accept(_sock, (sockaddr*)&addCli, &nlen);
#else
		_cSock = accept(_sock, (sockaddr*)&addCli, (socklen_t*)&nlen);
#endif
		if (_cSock==INVALID_SOCKET)
		{
			CellLog::Info("ERROR,等待接收客户端连接失败\n");
		}
		else
		{
			//inet_ntoa(addCli.sin_addr)//这个保留获取IP地址
			/*将新的客户端分配给客户端数量最少的cellServer*/
			addClienttoServer(new CellClient(_cSock));
		}
		return _cSock;
	}
	void addClienttoServer(CellClient* pClient)
	{
		//查找客户数量最少的CellServer消息处理对象
		auto pMinServer = _cellServer[0];
		//查找最小的客户数量最少的CellServer消息线程
		for (auto pClellServer:_cellServer)
		{
			if (pMinServer->getClientCount()>pClellServer->getClientCount())
			{
				pMinServer = pClellServer;
			}
		}
		pMinServer->addClient(pClient);
	}
	void Start(int nCellServerCount)
	{
		for (int i = 0; i < nCellServerCount; i++)
		{
			auto ser = new CellServer(i+1);
			_cellServer.push_back(ser);
			/*注册网络事件的接受对象*/
			ser->SetEventObj(this);
			/*启动消息处理线程*/
			ser->Start();

		}
		_thread.Start(nullptr, [this](CellThread* pThread)
			{
				OnRun(pThread);
			}, nullptr);
	}
	void Close()
	{
		_thread.Close();
		if (_sock!=INVALID_SOCKET)
		{
			for (auto s :_cellServer)
			{
				delete s;
			}
			_cellServer.clear();
#ifdef _WIN32
			//8 关闭套接字
			closesocket(_sock);
			WSACleanup();
#else
			//8 关闭套接字
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		}

	}
	
	//发送单发指定的Socket
	/*int SendData(DataHeader*header,SOCKET clientSock)
	{
		if (IsRun()&&header!=nullptr)
		{
			return send(clientSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}*/
	
	virtual void OnNetJoin(CellClient* pClient)
	{
		_ClientCount++;
	}

	/*客户端离开事件*/
	virtual void OnNetLeave(CellClient* pClient)
	{
		_ClientCount--;
	}
	virtual void OnNetMsg(CellServer*pCellServer,DataHeader* header, CellClient*pClient)
	{
		_MsgCount++;
	}
	
	virtual void OnNetRecv(CellClient* pClient)
	{
		_recvCount++;
	}

private:
	//处理网络消息
	//int nCount = 0;
	void OnRun(CellThread* pThread)
	{
		while(pThread->isRun())
		{
			time4msg();
			//伯克利 socket
			fd_set fdRead;
			//清除集合
			FD_ZERO(&fdRead);// fd_set 共有1024bit, 全部初始化为0
			//加入集合
			FD_SET(_sock, &fdRead);//将参数文件描述符fd对应的标志位,设置为1
			struct timeval _time;
			_time.tv_sec = 0;
			_time.tv_usec = 10;
			//nfds 是一个整数值 是指fd_set集合中所有的描述符socket 的范围,而不是数量,
		   //既是所有文件描述符最大值+1在windows无所谓 在linux是这样的
			int ret = select(_sock + 1, &fdRead, 0, 0, &_time);
			if (ret < 0)
			{
				CellLog::Info("Accept Select 任务结束1\n");
				_thread.Exit();
				break;
			}
			// 判断fd对应的标志位到底是0还是1, 返回值: fd对应的标志位的值, 0, 返回0, 1->返回1
			//有新连接
			//判断描述符(socket)是否在集合中
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);// 将参数文件描述符fd对应的标志位, 设置为0
				Accept();
				
			}
		}
	}
	/*计算并输出每秒收到的网络消息*/
	virtual void time4msg()
	{

		auto t1 = _time.getElapsedSecond();
		if (t1 >= 1.0)
		{
			CellLog::Info("thread<%d> time=%lf socket<%d> recv<%d> RecvCount=%d\n", _cellServer.size(), t1, _sock, (int)(_recvCount / t1), (int)(_MsgCount / t1));;
			_recvCount = 0;
			_MsgCount = 0;
			_time.update();
		}

	}
};
#endif
