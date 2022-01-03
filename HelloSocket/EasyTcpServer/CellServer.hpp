
#ifndef _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_

#include"Cell.hpp"
#include"INetEvent.hpp"
#include"CellClient.hpp"

#include<vector>
#include<map>

//网络消息发送任务  这些代码无效
//class CellSendMsg2ClientTask :public CellTask
//{
//	CellClient* _pClient;
//	DataHeader* _pHeader;
//public:
//	CellSendMsg2ClientTask(CellClient* pClient, DataHeader* header)
//	{
//		_pClient = pClient;
//		_pHeader = header;
//	}
//
//	//执行任务
//	void doTask()
//	{
//		_pClient->SendData(_pHeader);
//		delete _pHeader;
//	}
//};

//网络消息接收处理服务类
class CellServer
{
public:
	CellServer(SOCKET sock = INVALID_SOCKET)
	{
		_sock = sock;
		_pNetEvent = nullptr;
	}

	~CellServer()
	{
		Close();
		_sock = INVALID_SOCKET;
	}

	void SetEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	//关闭Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (auto iter : _clients)
			{
				closesocket(iter.second->Getsockfd());
				delete iter.second;
			}
			//关闭套节字closesocket
			closesocket(_sock);
#else
			for (auto iter : _clients)
			{
				close(iter.second->Getsockfd());
				delete iter.second;
			}
			//关闭套节字closesocket
			close(_sock);
#endif
			_clients.clear();
		}
	}

	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//处理网络消息
	//备份客户socket fd_set
	fd_set _fdRead_bak;
	//客户列表是否有变化
	bool _clients_change;
	SOCKET _maxSock;
	void OnRun()
	{
		_clients_change = true;
		while (isRun())
		{
			if (!_clientsBuff.empty())
			{//从缓冲队列里取出客户数据
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->Getsockfd()] = pClient; //将缓冲区队列的客户端信息加到正式队列中去
				}
				_clientsBuff.clear();
				_clients_change = true;
			}

			//如果没有需要处理的客户端，就跳过
			if (_clients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				//旧的时间戳
				_oldTime = CELLTime::getNowInMilliSec();
				continue;
			}

			//伯克利套接字 BSD socket
			fd_set fdRead;//描述符（socket） 集合
			//清理集合
			FD_ZERO(&fdRead);
			if (_clients_change)
			{
				_clients_change = false;
				//将描述符（socket）加入集合
				_maxSock = _clients.begin()->second->Getsockfd();
				for (auto iter : _clients)
				{
					FD_SET(iter.second->Getsockfd(), &fdRead);
					if (_maxSock < iter.second->Getsockfd())
					{
						_maxSock = iter.second->Getsockfd();
					}
				}
				memcpy(&_fdRead_bak, &fdRead, sizeof(fd_set));
			}
			else {
				memcpy(&fdRead, &_fdRead_bak, sizeof(fd_set));
			}

			///nfds 是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
			///既是所有文件描述符最大值+1 在Windows中这个参数可以写0
			int ret = select(_maxSock + 1, &fdRead, nullptr, nullptr, nullptr);
			if (ret < 0)
			{
				printf("select任务结束。\n");
				Close();
				return;
			}
			ReadData(fdRead);
			CheckTime();
		}
		

	}
	//旧的时间戳
	time_t _oldTime = CELLTime::getNowInMilliSec();

	/*检查时间  很影响性能*/
	void CheckTime()
	{
		//当前时间戳
		auto nowTime = CELLTime::getNowInMilliSec();
		auto dt = nowTime - _oldTime;
		_oldTime = nowTime;

		for (auto iter = _clients.begin(); iter !=_clients.end();)
		{
			//心跳检测
			if (iter->second->checkHeart(dt))
			{
				if (_pNetEvent)
					_pNetEvent->OnNetLeave(iter->second);
				_clients_change = true;
				delete iter->second;
				auto iterOld = iter;
				iter++;
				_clients.erase(iterOld);
				continue;
			}
			//定时发送检测
			iter->second->checkSend(dt);
			iter++;
		}
	}

	/*读出数据*/
	void ReadData(fd_set& fdRead)
	{
#ifdef _WIN32
		// 在windows中 fd_read.fd_count 认为是保留发生事件的socket的数量
		for (int n = 0; n < fdRead.fd_count; n++)
		{
			auto iter = _clients.find(fdRead.fd_array[n]);
			if (iter!=_clients.end())
			{
				if (-1==RecvData(iter->second))
				{
					if (_pNetEvent)
						_pNetEvent->OnNetLeave(iter->second);
					_clients_change = true;
					delete iter->second;
					closesocket(iter->first);
					_clients.erase(iter);
				}
			}
			else {
				printf("error. if (iter != _clients.end())\n");
			}

		}
#else
std::vector<CellClient*> temp;
			for (auto iter : _clients)
			{
				if (FD_ISSET(iter.second->Getsockfd(), &fdRead))
				{
					if (-1 == RecvData(iter.second))
					{
						if (_pNetEvent)
							_pNetEvent->OnNetLeave(iter.second);
						_clients_change = true;
						close(iter->first);
						temp.push_back(iter.second);
					}
				}
			}
			for (auto pClient : temp)
			{
				_clients.erase(pClient->Getsockfd());
				delete pClient;
			}
#endif
	}
	//接收数据 处理粘包 拆分包
	int RecvData(CellClient* pClient)
	{

		//接收客户端数据
		char* szRecv = pClient->GetmsgBuf() + pClient->GetLastPos();
		//数据存到szRecv中     第三个参数是可接收数据的最大长度
		int nLen = (int)recv(pClient->Getsockfd(), szRecv, (RECV_BUFF_SIZE)-pClient->GetLastPos(), 0);//返回值是接收的长度  revcz在mac返回值是long 建议强转int
		_pNetEvent->OnNetRecv(pClient);
		if (nLen <= 0)
		{
			//printf("客户端<Socket=%d>已退出，任务结束。\n", pClient->sockfd());
			return -1;
		}
		//将收取到的数据拷贝到消息缓冲区
		//memcpy(pClient->msgBuf() + pClient->getLastPos(), _szRecv, nLen);
		//消息缓冲区的数据尾部位置后移
		pClient->SetLastPos(pClient->GetLastPos() + nLen);

		//判断消息缓冲区的数据长度大于消息头netmsg_DataHeader长度
		while (pClient->GetLastPos() >= sizeof(DataHeader))
		{
			//这时就可以知道当前消息的长度
			DataHeader* header = (DataHeader*)pClient->GetmsgBuf();
			//判断消息缓冲区的数据长度大于消息长度
			if (pClient->GetLastPos() >= header->dataLength)
			{
				//消息缓冲区剩余未处理数据的长度
				int nSize = pClient->GetLastPos() - header->dataLength;
				//处理网络消息
				OnNetMsg(pClient, header);
				//将消息缓冲区剩余未处理数据前移
				memcpy(pClient->GetmsgBuf(), pClient->GetmsgBuf() + header->dataLength, nSize);
				//消息缓冲区的数据尾部位置前移
				pClient->SetLastPos(nSize);
			}
			else {
				//消息缓冲区剩余数据不够一条完整消息
				break;
			}
		}
		return 0;
	}

	//响应网络消息
	virtual void OnNetMsg(CellClient* pClient, DataHeader* header)
	{
		_pNetEvent->OnNetMsg(this, header,pClient);
	}

	void addClient(CellClient* pClient)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		//_mutex.lock();
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();
	}

	void Start()
	{
		_thread = std::thread(std::mem_fn(&CellServer::OnRun), this);
		_taskServer.Start();
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}

	/*暂时用不到 后面案例会经常使用*/
	//void addSendTask(CellClient* pClient, DataHeader* header)
	//{
	//	_taskServer.addTask([pClient, header]()
	//		{
	//			pClient->SendData(header);
	//			delete header;
	//		}
	//			);

	//}
private:
	SOCKET _sock;
	//正式客户队列
	std::map<SOCKET, CellClient*> _clients;
	//缓冲客户队列
	std::vector<CellClient*> _clientsBuff;
	//缓冲队列的锁
	std::mutex _mutex;
	std::thread _thread;
	//网络事件对象
	INetEvent* _pNetEvent;
	//
	CellTaskServer _taskServer;
};

#endif // !_CELL_SERVER_HPP_
