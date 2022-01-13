
#ifndef _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_

#include"Cell.hpp"
#include"INetEvent.hpp"
#include"CellClient.hpp"
#include "CellSemaphore.hpp"

#include<vector>
#include<map>
//网络消息接收处理服务类
class CellServer
{

private:
	void ClearClients()
	{
		for (auto iter:_clients)
		{
			delete iter.second;
		}
		_clients.clear();
		for (auto iter:_clientsBuff)
		{
			delete iter;
		}
		_clientsBuff.clear();
	}
public:
	CellServer(int id)
	{
		_id = id;

		_pNetEvent = nullptr;
		_taskServer._serverID = id;
	}

	~CellServer()
	{
		printf("CellServer%d.~CellServer1\n", _id);
		Close();
		printf("CellServer%d.~CellServer2\n", _id);
	}

	void SetEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	//关闭Socket
	void Close()

	{
		printf("CellServer%d.~Close1\n", _id);
		_taskServer.Close();
		_thread.Close();
		printf("CellServer%d.~Close2\n", _id);
	}
	void OnRun(CellThread*pThread)
	{
		while (pThread->isRun())
		{
			if (!_clientsBuff.empty())
			{//从缓冲队列里取出客户数据
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->Getsockfd()] = pClient; //将缓冲区队列的客户端信息加到正式队列中去
					pClient->_serverID = _id;
					if (_pNetEvent)
						_pNetEvent->OnNetJoin(pClient);
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
				_old_time = CELLTime::getNowInMilliSec();
				continue;
			}

			//伯克利套接字 BSD socket
			fd_set fdRead;//描述符（socket） 集合
			fd_set fdWrite; //描述符(socket)集合
			
			if (_clients_change)
			{
				_clients_change = false;

				//清理集合
				FD_ZERO(&fdRead);
				FD_ZERO(&fdWrite);
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
			memcpy(&fdWrite, &_fdRead_bak, sizeof(fd_set));
			///nfds 是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
			///既是所有文件描述符最大值+1 在Windows中这个参数可以写0
			timeval t{ 0,1 };
			int ret = select(_maxSock + 1, &fdRead, nullptr, nullptr, &t);
			if (ret < 0)
			{
				printf("select任务结束。\n");
				pThread->Exit();
				break;
			}
			ReadData(fdRead);
			WriteData(fdWrite);
			CheckTime();
		}
		printf("CellServer%d.OnRun\n", _id);

	}

	/*检查时间  很影响性能*/
	void CheckTime()
	{
		//当前时间戳
		auto nowTime = CELLTime::getNowInMilliSec();
		auto dt = nowTime - _old_time;
		_old_time = nowTime;

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
			//定时发送检测 现在由于发送的数据量比较大 不需要用到定时
			//iter->second->checkSend(dt);
			iter++;
		}
	}
	void OnClientLeave(CellClient*pcellClient)
	{
		if (_pNetEvent)
			_pNetEvent->OnNetLeave(pcellClient);
		_clients_change = true;
		delete pcellClient;
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
					OnClientLeave(iter->second);
					_clients.erase(iter);
				}
			}
			else {
				printf("error. if (iter != _clients.end())\n");
			}

		}
#else
		for (auto iter = _clients.begin(); iter != _clients.end(); )
		{
			if (FD_ISSET(iter->second->Getsockfd(), &fdRead))
			{
				if (-1 == RecvData(iter->second))
				{
					OnClientLeave(iter->second);
					auto iterOld = iter;
					iter++;
					_clients.erase(iterOld);
					continue;
				}
			}
			iter++;
		}
#endif
	}
	void WriteData(fd_set&fdWrite)
	{
#ifdef _WIN32
		// 在windows中 fd_read.fd_count 认为是保留发生事件的socket的数量
		for (int n = 0; n < fdWrite.fd_count; n++)
		{
			auto iter = _clients.find(fdWrite.fd_array[n]);
			if (iter != _clients.end())
			{
				if (-1 == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);
					_clients.erase(iter);
				}
			}

		}
#else
		for (auto iter = _clients.begin(); iter != _clients.end(); )
		{
			if (FD_ISSET(iter->second->Getsockfd(), &fdWrite))
			{
				if (-1 == iter->second->SendDataReal())
				{
					OnClientLeave(iter->second);
					auto iterOld = iter;
					iter++;
					_clients.erase(iterOld);
					continue;
				}
			}
			iter++;
		}
#endif
	}
	//接收数据 处理粘包 拆分包
	int RecvData(CellClient* pClient)
	{
		
		//接收客户端数据
		int nLen = pClient->RecvData();//返回值是接收的长度  revcz在mac返回值是long 建议强转int
	
		if (nLen <= 0)
		{
			//printf("客户端<Socket=%d>已退出，任务结束。\n", pClient->sockfd());
			return -1;
		}
		/*触发<接收网络数据>事件*/
		_pNetEvent->OnNetRecv(pClient);
	
		//解决粘包问题
		//循环 判断是否有消息需要处理
		while (pClient->hasMsg())
		{
			//处理网路哦消息
			OnNetMsg(pClient, pClient->front_msg());
			//移出消息队列(缓冲区)最前的一条数据
			pClient->Pop_front_msg();
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
		_taskServer.Start();
		_thread.Start(nullptr, [this](CellThread* pThread)
			{
				OnRun(pThread);
			}, [this](CellThread* pThread)
			{
				ClearClients();
			});
		
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}
private:
	//正式客户队列
	std::map<SOCKET, CellClient*> _clients;
	//缓冲客户队列
	std::vector<CellClient*> _clientsBuff;
	//缓冲队列的锁
	std::mutex _mutex;
	//网络事件对象
	INetEvent* _pNetEvent;
	//
	CellTaskServer _taskServer;


	//备份客户socket fd_set
	fd_set _fdRead_bak;//优化的地方

	//旧的时间戳
	time_t _old_time = CELLTime::getNowInMilliSec();

	//
	CellThread _thread;
	//
	int _id = -1;
	//
	SOCKET _maxSock;
	//客户列表是否有变化
	bool _clients_change = true;//优化的地方
};

#endif // !_CELL_SERVER_HPP_
