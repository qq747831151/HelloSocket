#ifndef _EasyTcpServer_HPP
#define  _EasyTcpServer_HPP
#ifdef _WIN32
#define FD_SETSIZE 2506
#define WIN32_LEAN_AND_MEAN//不影响 windows.h 和 WinSock2.h 前后顺序 
#define _WINSOCK_DEPRECATED_NO_WARNINGS //这个用于 inet_ntoa   可以在右击项目属性 C/C++ 预处理里面 预处理定义添加
#include <windows.h>
#include <WinSock2.h>
/*为了可以在其他平台也可以使用 右键项目属性 选择链接器 附加依赖项 将ws2_32.lib 添加进去就行 这样就不需要 下面这些 */
#pragma  comment(lib,"ws2_32.lib")
#else
#include<unistd.h> //uni std
#include<arpa/inet.h>
#include<string.h>
#include<sys/select.h>
#include<pthread.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include"CELLTimestamp.hpp"

#include "MessAgeHeader.hpp"
#include <thread>
#include <functional>
#include <algorithm>
#include <atomic>
#include"mutex"
#ifndef RECV_BUFF_SIZE
//缓冲区区域最小单元大小 
#define RECV_BUFF_SIZE 10240
#endif 
#define _CellServer_THREAD_COUNT 4

/*存储客户端的信息*/
class ClientScoket
{
public:
	ClientScoket(SOCKET sock)
	{
		_sockfd = sock;
		lastPos = 0;
		memset(szMsg, 0, sizeof(szMsg));
	}
	SOCKET Getsockfd() {
		return _sockfd;
	}
	char* GetmsgBuf() {
		return szMsg;
	}
	int GetLastPos() {
		return lastPos;
	}
	void SetLastPos(int pos) {
		lastPos = pos;
	}

private:
	//第二缓冲区 消息缓冲区
	char szMsg[RECV_BUFF_SIZE * 10];
	//消息缓冲的数据尾部的位置
	int lastPos;
	SOCKET _sockfd;

};
class INetEvent
{
public:
	/*客户端离开事件*/
	virtual void OnLeave(ClientScoket* pClient) = 0;//纯虚函数
	/**/
	virtual void OnNetMsg(DataHeader* header, SOCKET _clientSock) = 0;//纯虚函数

private:

};


/*处理客户端消息*/
class CellServer
{
private:
	std::mutex _mutex;
	SOCKET _sock;
	//正式客户端队列
	std::vector<ClientScoket*>_clients;
	//缓冲客户端队列
	std::vector<ClientScoket*>_clientsBuf;
	std::thread* _Pthread;
	INetEvent* _pNetevt;
public:
	std::atomic_int _recvCount;
	CellServer(SOCKET sock=INVALID_SOCKET)
	{
		_sock = sock;
		_Pthread = nullptr;
		_recvCount = 0;
		_pNetevt = nullptr;

	}
	~CellServer()
	{
		Close();
		_sock = INVALID_SOCKET;
	}
	void SetEventObj(INetEvent*pNet)
	{
		_pNetevt = pNet;
	}
	//关闭Socket
	void Close() {
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (int n = _clients.size() - 1; n >= 0; n--)
			{
				closesocket(_clients[n]->Getsockfd());
				delete _clients[n];

			}
			closesocket(_sock);
			//清除windows socket环境
			WSACleanup();
#else
			for (int n = _clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->Getsockfd());
				delete _clients[n];
			}
			close(_sock);
#endif 
			_clients.clear();
		}
	}
	/*是否工作中*/
	bool IsRun() {
		return _sock != INVALID_SOCKET;
	}
	//处理网络消息
	bool OnRun()
	{
		while (IsRun())
		{
			//缓冲区
			if (_clientsBuf.size()>0)
			{
				//从缓冲区队列里取出客户数据
				std::lock_guard<std::mutex> lock(_mutex);//锁住
				for (auto pClient : _clientsBuf)
				{
					_clients.push_back(pClient);
				}
				_clientsBuf.clear();
			}
			//如果没有需要处理的客户端,就跳过
			if (_clients.empty())
			{
				std::chrono::microseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			//伯克利 socket
			fd_set fdRead;
			//清除集合
			FD_ZERO(&fdRead);// fd_set 共有1024bit, 全部初始化为0
			//将描述符（socket）加入集合
			SOCKET maxSock =_clients[0]->Getsockfd();
			for (int i = (int)_clients.size() - 1; i >= 0; i--)
			{
				FD_SET(_clients[i]->Getsockfd(), &fdRead);//可以放只读 另外两个也可以放
				if (_clients[i]->Getsockfd() > maxSock)
				{
					maxSock = _clients[i]->Getsockfd();
				}
			}
			///nfds 是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
			///既是所有文件描述符最大值+1 在Windows中这个参数可以写0
			int ret = select(maxSock + 1, &fdRead, nullptr, nullptr, nullptr);
			if (ret < 0)
			{
				printf("select任务结束。\n");
				Close();
				return false;
			}
			for (int i = (int)_clients.size() - 1; i >= 0; i--)
			{
				if (FD_ISSET(_clients[i]->Getsockfd(),&fdRead))
				{
					if (-1 == RecvData(_clients[i]))
					{
						std::vector<ClientScoket*>::iterator iter = _clients.begin() + i;;//i是删除的迭代器
						if (iter != _clients.end())
						{
							if (_pNetevt)
							{
								_pNetevt->OnLeave(_clients[i]);
							}
							delete _clients[i];
							_clients.erase(iter);//删除
						}
					}
				}
			}
		}
	}
	//缓冲区
	char szRecv[RECV_BUFF_SIZE] = {};
	//接受数据 处理粘包 拆分包
	int RecvData(ClientScoket* clientSock)
	{
		//5.接受客户端请求数据
		//数据存到szRecv中     第三个参数是可接收数据的最大长度
		int nlen = (int)recv(clientSock->Getsockfd(), szRecv, RECV_BUFF_SIZE, 0);//返回值是接收的长度  revcz在mac返回值是long 建议强转int
		if (nlen <= 0)
		{
			printf("客户端<socket=%d>退出,任务结束\n", clientSock->Getsockfd());
			return -1;
		}

		//将收取打的数据拷贝到消息缓冲区
		memcpy(clientSock->GetmsgBuf() + clientSock->GetLastPos(), szRecv, nlen);
		//消息缓冲区的数据尾部位置后移
		clientSock->SetLastPos(clientSock->GetLastPos() + nlen);

		//判断粘包
		//判断消息缓冲区的数据长度是否大于消息头DataHeader长度
		while (clientSock->GetLastPos() >= sizeof(DataHeader))
		{
			//这时就可以知道当前消息的长度
			DataHeader* header = (DataHeader*)clientSock->GetmsgBuf();
			//判断消息缓冲的数据长度大于消息长度
			if (clientSock->GetLastPos() >= header->dataLength)
			{
				//剩余未处理的消息缓冲区数据的长度
				int nsize = clientSock->GetLastPos() - header->dataLength;
				//处理网络消息
				OnNetMsg(clientSock->Getsockfd(),header);
				//将消息缓冲区剩余未处理数据前移
				memcpy(clientSock->GetmsgBuf(), clientSock->GetmsgBuf() + header->dataLength, nsize);
				//消息缓冲区的数据尾部位置前移
				clientSock->SetLastPos(nsize);
			}
			else
			{
				//消息缓冲区剩余数据 不够一条完整的数据
				break;
			}
		}
		return 0;
	}

	//响应网络消息
	virtual void OnNetMsg(SOCKET cSock, DataHeader* header)
	{
		_recvCount++;
		_pNetevt->OnNetMsg( header, cSock);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{

			Login* login = (Login*)header;
			//printf("收到客户端<Socket=%d>请求：CMD_LOGIN,数据长度：%d,userName=%s PassWord=%s\n", cSock, login->dataLength, login->userName, login->PassWord);
			//忽略判断用户密码是否正确的过程
			//LoginResult ret;
			//SendData(cSock, &ret);
		}
		break;
		case CMD_LOGINOUT:
		{
			LoginOut* logout = (LoginOut*)header;
			//printf("收到客户端<Socket=%d>请求：CMD_LOGOUT,数据长度：%d,userName=%s \n", cSock, logout->dataLength, logout->userName);
			//忽略判断用户密码是否正确的过程
			//LogoutResult ret;
			//SendData(cSock, &ret);
		}
		break;
		default:
		{
			printf("<socket=%d>收到未定义消息,数据长度：%d\n", cSock, header->dataLength);
			//DataHeader ret;
			//SendData(cSock, &ret);
		}
		break;
		}
	}

	void addClient(ClientScoket* pClient)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		//_mutex.lock();
		_clientsBuf.push_back(pClient);
		//_mutex.unlock();
	}

	void Start()
	{
		_Pthread= new std::thread(std::mem_fun(&CellServer::OnRun), this);
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuf.size();
	}

private:

};

class EasyTcpServer:public INetEvent
{
private:
	SOCKET _sock;
	std::vector<ClientScoket*> g_clients;
	std::vector<CellServer*>_cellServer;
	CELLTimestamp _time;
	//int _recvCount;
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;//无效地址
		//_recvCount = 0;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}
	//初始化
	void InitSocket()
	{
#ifdef _WIN32
		/*启动socket网络环境 2.x环境*/
		WORD ver = MAKEWORD(2, 2);//版本号
		WSADATA dat;
		WSAStartup(ver, &dat);//动态库需要写上那个lib
#endif

		if (_sock!=INVALID_SOCKET)
		{
			printf("<Socket=%d>关闭之前的旧链接\n", _sock);
		}
	//1.建立Socket API 建立简易TC服务端
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (_sock == INVALID_SOCKET)
		{
			printf("ERROR,<socket=%d>建立socket失败...\n", _sock);
		}
		else
		{
			printf("TURE,<socket=%d>建立socket成功.....\n", _sock);
		}

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
			printf("ERROR,绑定用于接受客户端连接的网络端口失败...\n");
		}
		else
		{
			printf("TURE,绑定用于接受客户端连接的网络端口成功.....\n");
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
			printf("ERROR,监听网络端口失败...\n");
		}
		else
		{
			printf("TURE,监听网络端口成功.....\n");
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
			printf("ERROR,等待接收客户端连接失败\n");
		}
		else
		{
			printf("TURE,等待接收客户端<socket=%d>连接成功\n",_cSock);
			//如果有新客户端加入,就向其他现有的客户端发送信息
			//LoginNewUser newUser;
			//SendData2All(&newUser);
			//printf("新客户端加入:socket=%d IP=%s\n", (int)_cSock, inet_ntoa(addCli.sin_addr));
			addClienttoServer(new ClientScoket(_cSock));
		}
		return _cSock;
	}
	void addClienttoServer(ClientScoket* pClient)
	{
		g_clients.push_back(pClient);
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
	void Start()
	{
		for (int i = 0; i < _CellServer_THREAD_COUNT; i++)
		{
			auto ser = new CellServer(_sock);
			_cellServer.push_back(ser);
			ser->SetEventObj(this);
			ser->Start();

		}
	}
	void Close()
	{
		if (_sock!=INVALID_SOCKET)
		{
#ifdef _WIN32
			for (int i = g_clients.size() - 1; i >= 0; i--)
			{
				closesocket(g_clients[i]->Getsockfd());
				delete g_clients[i];
			}
			//8 关闭套接字
			closesocket(_sock);
			WSACleanup();
#else
			for (int i = g_clients.size() - 1; i >= 0; i--)
			{
				close(g_clients[i]->Getsockfd());
				delete g_clients[i];
			}
			//8 关闭套接字
			close(_sock);
#endif
		}
		g_clients.clear();

	}
	//处理网络消息
	//int nCount = 0;
	bool OnRun()
	{
		if (IsRun())
		{
			time4msg();
			//伯克利 socket
			fd_set fdRead;
			//清除集合
			FD_ZERO(&fdRead);// fd_set 共有1024bit, 全部初始化为0
			//加入集合
			FD_SET(_sock, &fdRead);//将参数文件描述符fd对应的标志位,设置为1
			struct timeval _time;
			_time.tv_sec = 1;
			_time.tv_usec = 0;
			//nfds 是一个整数值 是指fd_set集合中所有的描述符socket 的范围,而不是数量,
	       //既是所有文件描述符最大值+1在windows无所谓 在linux是这样的
			int ret = select(_sock + 1, &fdRead, 0,0, &_time);
			if (ret < 0)
			{
				printf("客户端已退出,任务结束\n");
				Close();
				return false;
			}
			// 判断fd对应的标志位到底是0还是1, 返回值: fd对应的标志位的值, 0, 返回0, 1->返回1
			//有新连接
			//判断描述符(socket)是否在集合中
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);// 将参数文件描述符fd对应的标志位, 设置为0
				Accept();
				return true;
			}
			return true;
		}
		return false;
	}
	/*是否工作中*/
	bool IsRun() {
		return _sock != INVALID_SOCKET;
	}
	//响应网络消息
	virtual void time4msg()
	{
		//_recvCount++;
		auto t1 = _time.getElapsedSecond();
		if (t1 >= 1.0)
		{
			int recvCount = 0;
			for ( auto ser:_cellServer)
			{
				recvCount += ser->_recvCount;
				ser->_recvCount = 0;
			}
			printf("thread<%d> time=%lf socket<%d>c RecvCount=%d\n", _cellServer.size(), t1, _sock, (int)(recvCount / t1));;
			_time.update();
		}
		
	}
	//发送单发指定的Socket
	int SendData(DataHeader*header,SOCKET clientSock)
	{
		if (IsRun()&&header!=nullptr)
		{
			return send(clientSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	//发送群发数据
	void SendData2All(DataHeader* header)
	{
		if (IsRun() && header != NULL)
		{
			//如果有新客户端加入 就向其他现有的客户端发送
			for (int n = g_clients.size() - 1; n >= 0; n--)
			{
				SendData(header, g_clients[n]->Getsockfd());
			}

		}
	}
	/*客户端离开事件*/
	virtual void OnLeave(ClientScoket* pClient)
	{
		for (int i = g_clients.size()-1; i >=0 ;i--)
		{
			if (g_clients[i]==pClient)
			{
				auto iter = g_clients.begin() + i;
				if (iter != g_clients.end())
					g_clients.erase(iter);
			}
		}
	}
	virtual void OnNetMsg(DataHeader* header, SOCKET _clientSock) {}

private:

};
#endif
