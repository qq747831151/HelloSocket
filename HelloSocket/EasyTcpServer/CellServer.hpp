
#ifndef _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_

#include"Cell.hpp"
#include"INetEvent.hpp"
#include"CellClient.hpp"

#include<vector>
#include<map>

//������Ϣ��������  ��Щ������Ч
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
//	//ִ������
//	void doTask()
//	{
//		_pClient->SendData(_pHeader);
//		delete _pHeader;
//	}
//};

//������Ϣ���մ��������
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

	//�ر�Socket
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
			//�ر��׽���closesocket
			closesocket(_sock);
#else
			for (auto iter : _clients)
			{
				close(iter.second->Getsockfd());
				delete iter.second;
			}
			//�ر��׽���closesocket
			close(_sock);
#endif
			_clients.clear();
		}
	}

	//�Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//����������Ϣ
	//���ݿͻ�socket fd_set
	fd_set _fdRead_bak;
	//�ͻ��б��Ƿ��б仯
	bool _clients_change;
	SOCKET _maxSock;
	void OnRun()
	{
		_clients_change = true;
		while (isRun())
		{
			if (!_clientsBuff.empty())
			{//�ӻ��������ȡ���ͻ�����
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->Getsockfd()] = pClient; //�����������еĿͻ�����Ϣ�ӵ���ʽ������ȥ
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
				_oldTime = CELLTime::getNowInMilliSec();
				continue;
			}

			//�������׽��� BSD socket
			fd_set fdRead;//��������socket�� ����
			//������
			FD_ZERO(&fdRead);
			if (_clients_change)
			{
				_clients_change = false;
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

			///nfds ��һ������ֵ ��ָfd_set����������������(socket)�ķ�Χ������������
			///���������ļ����������ֵ+1 ��Windows�������������д0
			int ret = select(_maxSock + 1, &fdRead, nullptr, nullptr, nullptr);
			if (ret < 0)
			{
				printf("select���������\n");
				Close();
				return;
			}
			ReadData(fdRead);
			CheckTime();
		}
		

	}
	//�ɵ�ʱ���
	time_t _oldTime = CELLTime::getNowInMilliSec();

	/*���ʱ��  ��Ӱ������*/
	void CheckTime()
	{
		//��ǰʱ���
		auto nowTime = CELLTime::getNowInMilliSec();
		auto dt = nowTime - _oldTime;
		_oldTime = nowTime;

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
			//��ʱ���ͼ��
			iter->second->checkSend(dt);
			iter++;
		}
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
	//�������� ����ճ�� ��ְ�
	int RecvData(CellClient* pClient)
	{

		//���տͻ�������
		char* szRecv = pClient->GetmsgBuf() + pClient->GetLastPos();
		//���ݴ浽szRecv��     �����������ǿɽ������ݵ���󳤶�
		int nLen = (int)recv(pClient->Getsockfd(), szRecv, (RECV_BUFF_SIZE)-pClient->GetLastPos(), 0);//����ֵ�ǽ��յĳ���  revcz��mac����ֵ��long ����ǿתint
		_pNetEvent->OnNetRecv(pClient);
		if (nLen <= 0)
		{
			//printf("�ͻ���<Socket=%d>���˳������������\n", pClient->sockfd());
			return -1;
		}
		//����ȡ�������ݿ�������Ϣ������
		//memcpy(pClient->msgBuf() + pClient->getLastPos(), _szRecv, nLen);
		//��Ϣ������������β��λ�ú���
		pClient->SetLastPos(pClient->GetLastPos() + nLen);

		//�ж���Ϣ�����������ݳ��ȴ�����Ϣͷnetmsg_DataHeader����
		while (pClient->GetLastPos() >= sizeof(DataHeader))
		{
			//��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
			DataHeader* header = (DataHeader*)pClient->GetmsgBuf();
			//�ж���Ϣ�����������ݳ��ȴ�����Ϣ����
			if (pClient->GetLastPos() >= header->dataLength)
			{
				//��Ϣ������ʣ��δ�������ݵĳ���
				int nSize = pClient->GetLastPos() - header->dataLength;
				//����������Ϣ
				OnNetMsg(pClient, header);
				//����Ϣ������ʣ��δ��������ǰ��
				memcpy(pClient->GetmsgBuf(), pClient->GetmsgBuf() + header->dataLength, nSize);
				//��Ϣ������������β��λ��ǰ��
				pClient->SetLastPos(nSize);
			}
			else {
				//��Ϣ������ʣ�����ݲ���һ��������Ϣ
				break;
			}
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
		_thread = std::thread(std::mem_fn(&CellServer::OnRun), this);
		_taskServer.Start();
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}

	/*��ʱ�ò��� ���永���ᾭ��ʹ��*/
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
	//��ʽ�ͻ�����
	std::map<SOCKET, CellClient*> _clients;
	//����ͻ�����
	std::vector<CellClient*> _clientsBuff;
	//������е���
	std::mutex _mutex;
	std::thread _thread;
	//�����¼�����
	INetEvent* _pNetEvent;
	//
	CellTaskServer _taskServer;
};

#endif // !_CELL_SERVER_HPP_
