
#ifndef _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_

#include"Cell.hpp"
#include"INetEvent.hpp"
#include"CellClient.hpp"
#include "CellSemaphore.hpp"

#include<vector>
#include<map>
//������Ϣ���մ��������
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

	//�ر�Socket
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
			{//�ӻ��������ȡ���ͻ�����
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->Getsockfd()] = pClient; //�����������еĿͻ�����Ϣ�ӵ���ʽ������ȥ
					pClient->_serverID = _id;
					if (_pNetEvent)
						_pNetEvent->OnNetJoin(pClient);
				}
				_clientsBuff.clear();
				_clients_change = true;
			}

			//���û����Ҫ����Ŀͻ��ˣ�������
			if (_clients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				//�ɵ�ʱ���
				_old_time = CELLTime::getNowInMilliSec();
				continue;
			}

			//�������׽��� BSD socket
			fd_set fdRead;//��������socket�� ����
			fd_set fdWrite; //������(socket)����
			
			if (_clients_change)
			{
				_clients_change = false;

				//������
				FD_ZERO(&fdRead);
				FD_ZERO(&fdWrite);
				//����������socket�����뼯��
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
			///nfds ��һ������ֵ ��ָfd_set����������������(socket)�ķ�Χ������������
			///���������ļ����������ֵ+1 ��Windows�������������д0
			timeval t{ 0,1 };
			int ret = select(_maxSock + 1, &fdRead, nullptr, nullptr, &t);
			if (ret < 0)
			{
				printf("select���������\n");
				pThread->Exit();
				break;
			}
			ReadData(fdRead);
			WriteData(fdWrite);
			CheckTime();
		}
		printf("CellServer%d.OnRun\n", _id);

	}

	/*���ʱ��  ��Ӱ������*/
	void CheckTime()
	{
		//��ǰʱ���
		auto nowTime = CELLTime::getNowInMilliSec();
		auto dt = nowTime - _old_time;
		_old_time = nowTime;

		for (auto iter = _clients.begin(); iter !=_clients.end();)
		{
			//�������
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
			//��ʱ���ͼ�� �������ڷ��͵��������Ƚϴ� ����Ҫ�õ���ʱ
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
	/*��������*/
	void ReadData(fd_set& fdRead)
	{
#ifdef _WIN32
		// ��windows�� fd_read.fd_count ��Ϊ�Ǳ��������¼���socket������
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
		// ��windows�� fd_read.fd_count ��Ϊ�Ǳ��������¼���socket������
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
	//�������� ����ճ�� ��ְ�
	int RecvData(CellClient* pClient)
	{
		
		//���տͻ�������
		int nLen = pClient->RecvData();//����ֵ�ǽ��յĳ���  revcz��mac����ֵ��long ����ǿתint
	
		if (nLen <= 0)
		{
			//printf("�ͻ���<Socket=%d>���˳������������\n", pClient->sockfd());
			return -1;
		}
		/*����<������������>�¼�*/
		_pNetEvent->OnNetRecv(pClient);
	
		//���ճ������
		//ѭ�� �ж��Ƿ�����Ϣ��Ҫ����
		while (pClient->hasMsg())
		{
			//������·Ŷ��Ϣ
			OnNetMsg(pClient, pClient->front_msg());
			//�Ƴ���Ϣ����(������)��ǰ��һ������
			pClient->Pop_front_msg();
		}
		return 0;
	}

	//��Ӧ������Ϣ
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
	//��ʽ�ͻ�����
	std::map<SOCKET, CellClient*> _clients;
	//����ͻ�����
	std::vector<CellClient*> _clientsBuff;
	//������е���
	std::mutex _mutex;
	//�����¼�����
	INetEvent* _pNetEvent;
	//
	CellTaskServer _taskServer;


	//���ݿͻ�socket fd_set
	fd_set _fdRead_bak;//�Ż��ĵط�

	//�ɵ�ʱ���
	time_t _old_time = CELLTime::getNowInMilliSec();

	//
	CellThread _thread;
	//
	int _id = -1;
	//
	SOCKET _maxSock;
	//�ͻ��б��Ƿ��б仯
	bool _clients_change = true;//�Ż��ĵط�
};

#endif // !_CELL_SERVER_HPP_
