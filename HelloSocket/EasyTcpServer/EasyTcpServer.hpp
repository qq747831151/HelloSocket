#ifndef _EasyTcpServer_HPP
#define  _EasyTcpServer_HPP
#ifdef _WIN32
#define FD_SETSIZE 2506
#define WIN32_LEAN_AND_MEAN//��Ӱ�� windows.h �� WinSock2.h ǰ��˳�� 
#define _WINSOCK_DEPRECATED_NO_WARNINGS //������� inet_ntoa   �������һ���Ŀ���� C/C++ Ԥ�������� Ԥ���������
#include <windows.h>
#include <WinSock2.h>
/*Ϊ�˿���������ƽ̨Ҳ����ʹ�� �Ҽ���Ŀ���� ѡ�������� ���������� ��ws2_32.lib ��ӽ�ȥ���� �����Ͳ���Ҫ ������Щ */
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
#include<mutex>
#include <map>

#ifndef RECV_BUFF_SIZE 
//������������С��Ԫ��С  10240��10k
#define RECV_BUFF_SIZE 10240*5
#define SEND_BUFF_SIZE RECV_BUFF_SIZE
#endif 

/*�ͻ�����������*/
class ClientScoket
{
public:
	ClientScoket(SOCKET sock)
	{
		_sockfd = sock;
		lastPos = 0;
		lastSendPos = 0;
		memset(szMsg, 0, sizeof(szMsg));
		memset(szSend, 0, sizeof(szSend));
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
	//��������
	int SendData(DataHeader*header)
	{
		int ret = SOCKET_ERROR;
		//Ҫ���͵����ݳ���
		int nSendLen = header->dataLength;
		//���͵�����
		const char* pSendData = (const char *)header;
		while (true)
		{
           //���� ��������
			if (nSendLen+lastSendPos>=SEND_BUFF_SIZE)
			{
				//����ɿ��������ݳ���
				int nCopylen = SEND_BUFF_SIZE - lastSendPos;
				//��������
				memcpy(szSend + lastSendPos, pSendData, nCopylen);
				//����ʣ������λ��
				pSendData += nCopylen;
				//����ʣ�����ݳ���
				nSendLen -= nSendLen;
				////��������
				ret = send(_sockfd, szSend, SEND_BUFF_SIZE, 0);
				//����β��λ������0
				lastSendPos = 0;
				//���ʹ���
				if (SOCKET_ERROR==ret)
				{
					return ret;
				}
			}
			else {
                //��Ҫ���͵����� ����������β��
				memcpy(szSend + lastSendPos, pSendData, nSendLen);
				//��������β��λ��
				lastSendPos+=nSendLen;
				break;
			}

		}
		return ret;
	}
private:
	//�ڶ������� ��Ϣ������
	char szMsg[RECV_BUFF_SIZE];
	//��Ϣ���������β����λ��
	int lastPos;
	//�ڶ������� ���ͻ�����
	char szSend[SEND_BUFF_SIZE];
	//���ͻ��������β��λ��
	int lastSendPos;

	SOCKET _sockfd;


};
class INetEvent
{
public:
	/*�ͻ����뿪�¼�*/
	virtual void OnNetLeave(ClientScoket* pClient) = 0;//���麯��
	/*�ͻ�����Ϣ�¼�*/
	virtual void OnNetMsg(DataHeader* header, ClientScoket*pClient) = 0;//���麯��
	/*�ͻ��˼����¼�*/
	virtual void OnNetJoin(ClientScoket* pClient) = 0;
	/*Recv�¼�*/
	virtual void OnNetRecv(ClientScoket* pClient) = 0;//�麯��


private:

};


/*����ͻ�����Ϣ*/
class CellServer
{
private:
	/*������е���*/
	std::mutex _mutex;
	SOCKET _sock;
	//��ʽ�ͻ��˶���
	std::map<SOCKET,ClientScoket*>_clients;
	//����ͻ��˶���
	std::vector<ClientScoket*>_clientsBuf;
	std::thread* _Pthread;
	/*�����¼�����*/
	INetEvent* _pNetevt;
public:
	CellServer(SOCKET sock=INVALID_SOCKET)
	{
		_sock = sock;
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
	//�ر�Socket
	void Close() {
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (int n =(int) _clients.size() - 1; n >= 0; n--)
			{
				closesocket(_clients[n]->Getsockfd());
				delete _clients[n];

			}
			/*�ر��׽���*/
			closesocket(_sock);

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
	/*�Ƿ�����*/
	bool IsRun() {
		return _sock != INVALID_SOCKET;
	}
	//����������Ϣ
	//���ݿͻ�socket fd_set
	fd_set fdRead_bak;
	//�ͻ��б��Ƿ��б仯
	bool _client_change;//�Ż��ĵط�
	bool OnRun()
	{
		_client_change = true;
		SOCKET maxSocket;
		while (IsRun())
		{
			//������
			if (_clientsBuf.size()>0)
			{
				//�ӻ�����������ȡ���ͻ�����
				std::lock_guard<std::mutex> lock(_mutex);//��ס
				for (auto pClient : _clientsBuf)
				{
					_clients[pClient->Getsockfd()]=pClient;/*�����������еĿͻ�����Ϣ�ӵ���ʽ������ȥ*/
				}
				_clientsBuf.clear();
				_client_change = true;
			}
			//���û����Ҫ����Ŀͻ���,������
			if (_clients.empty())
			{
				std::chrono::microseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			//������ socket
			fd_set fdRead;
			//�������
			FD_ZERO(&fdRead);// fd_set ����1024bit, ȫ����ʼ��Ϊ0
			//����������socket�����뼯��
			if (_client_change)
			{
				_client_change = false;
				maxSocket = _clients.begin()->second->Getsockfd();
				for (auto iter:_clients)
				{
					FD_SET(iter.second->Getsockfd(), &fdRead);
					if (iter.second->Getsockfd()>maxSocket)
					{
						maxSocket = iter.second->Getsockfd();
					}
				}
				memcpy(&fdRead_bak, &fdRead, sizeof(fd_set));
			}
			else
			{
				memcpy( &fdRead, &fdRead_bak,sizeof(fd_set));
			}
			///nfds ��һ������ֵ ��ָfd_set����������������(socket)�ķ�Χ������������
			///���������ļ����������ֵ+1 ��Windows�������������д0
			struct timeval time;
			time.tv_sec = 0;
			time.tv_usec = 0;
			int ret = select(maxSocket+1, &fdRead, nullptr, nullptr, &time);
			if (ret < 0)
			{
				printf("select�������2.\n");
				Close();
				return false;
			}
			else if(ret==0)
			{
				continue;
			}
#ifdef _WIN32
			for (int i = 0; i < fdRead.fd_count; i++)
			{
				auto iter = _clients.find(fdRead.fd_array[i]);
				if (iter!=_clients.end())
				{
					if (-1==RecvData(iter->second))
					{
						if (_pNetevt)
						{
							_pNetevt->OnNetLeave(iter->second);
						}
						_client_change = true;
						_clients.erase(iter->first);
					}
				}
				else
				{
					printf("error:iter != _clients.end()");
				}
			}
#else
			std::vector<ClientScoket*> temp;
			// ͨ��, �пͻ��˷������ݹ���
			for (auto iter : _clients)
			{
				if (FD_ISSET(iter.second->Getsockfd(), &fdRead))
				{
					if (-1 == RecvData(iter->second))
					{

						if (_pNetevt)
							_pNetevt->OnNetLeave(iter->second);
						_client_change = true;
						temp.push_back(iter->second);

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
	}
	//�������� ����ճ�� ��ְ�
	int RecvData(ClientScoket* clientSock)
	{

		//5.���ܿͻ�����������
		char* szRecv = clientSock->GetmsgBuf() + clientSock->GetLastPos();
		_pNetevt->OnNetRecv(clientSock);
		//���ݴ浽szRecv��     �����������ǿɽ������ݵ���󳤶�
		int nlen = (int)recv(clientSock->Getsockfd(), szRecv, (RECV_BUFF_SIZE)-clientSock->GetLastPos(), 0);//����ֵ�ǽ��յĳ���  revcz��mac����ֵ��long ����ǿתint
		if (nlen <= 0)
		{
			return -1;
		}

		//����ȡ������ݿ�������Ϣ������
		//memcpy(clientSock->GetmsgBuf() + clientSock->GetLastPos(), szRecv, nlen);
		//��Ϣ������������β��λ�ú���
		clientSock->SetLastPos(clientSock->GetLastPos() + nlen);

		//�ж�ճ��
		//�ж���Ϣ�����������ݳ����Ƿ������ϢͷDataHeader����
		while (clientSock->GetLastPos() >= sizeof(DataHeader))
		{
			//��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
			DataHeader* header = (DataHeader*)clientSock->GetmsgBuf();
			//�ж���Ϣ��������ݳ��ȴ�����Ϣ����
			if (clientSock->GetLastPos() >= header->dataLength)
			{
				//ʣ��δ�������Ϣ���������ݵĳ���
				int nsize = clientSock->GetLastPos() - header->dataLength;
				//����������Ϣ
				OnNetMsg(clientSock,header);
				//����Ϣ������ʣ��δ��������ǰ��
				memcpy(clientSock->GetmsgBuf(), clientSock->GetmsgBuf() + header->dataLength, nsize);
				//��Ϣ������������β��λ��ǰ��
				clientSock->SetLastPos(nsize);
			}
			else
			{
				//��Ϣ������ʣ������ ����һ������������
				break;
			}
		}
		return 0;
	}

	//��Ӧ������Ϣ
	virtual void OnNetMsg(ClientScoket*pClient, DataHeader* header)
	{
		_pNetevt->OnNetMsg( header, pClient);
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
		_Pthread= new std::thread(std::mem_fn(&CellServer::OnRun), this);
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
	/*��Ϣ�������,�ڲ��ᴴ���߳�*/
	std::vector<CellServer*>_cellServer;
	/*ÿ����Ϣ��ʱ*/
	CELLTimestamp _time;
protected:
	std::atomic_int _recvCount;//�յ���Ϣ����
	std::atomic_int  _ClientCount;//�ͻ��˼���
	std::atomic_int  _MsgCount;//SOCKET recv��������
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;//��Ч��ַ
		_recvCount = 0;
		_ClientCount = 0;
		_MsgCount = 0;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}
	//��ʼ��
	void InitSocket()
	{
#ifdef _WIN32
		/*����socket���绷�� 2.x����*/
		WORD ver = MAKEWORD(2, 2);//�汾��
		WSADATA dat;
		WSAStartup(ver, &dat);//��̬����Ҫд���Ǹ�lib
#endif

		if (_sock!=INVALID_SOCKET)
		{
			printf("<Socket=%d>�ر�֮ǰ�ľ�����\n", _sock);
			Close();
		}
	//1.����Socket API ��������TC�����
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (_sock == INVALID_SOCKET)
		{
			printf("ERROR,<socket=%d>����socketʧ��...\n", _sock);
		}
		else
		{
			printf("TURE,<socket=%d>����socket�ɹ�.....\n", _sock);
		}

	}
	int Bind(const char *ip,unsigned short port)
	{

		//2.�� ���ڽ��ܿͻ������ӵ�����˿�
		sockaddr_in _sin = {  };
		_sin.sin_family = AF_INET;//ipv4
		_sin.sin_port = htons(port);//���ֽ���Ӧ���������ֽ���
#ifdef _WIN32
		if (ip)
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);;
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = ADDR_ANY;//��ȡIP�ò��������ں�
		}
	
	//_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");�����Ŀֻ������ʹ�õĻ�����ʹ��127
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
			printf("ERROR,�����ڽ��ܿͻ������ӵ�����˿�ʧ��...\n");
		}
		else
		{
			printf("TURE,�����ڽ��ܿͻ������ӵ�����˿ڳɹ�.....\n");
		}
		return ret;
	}
	//�����˿ں�
	int Listen(int n)
	{
		//3.��������˿�
		int ret = listen(_sock, n);
		if (ret == SOCKET_ERROR)
		{
			printf("ERROR,��������˿�ʧ��...\n");
		}
		else
		{
			printf("TURE,��������˿ڳɹ�.....\n");
		}
		return ret;
	}
	//���տͻ�������
	SOCKET Accept()
	{
		//4.�ȴ����տͻ�������
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
			printf("ERROR,�ȴ����տͻ�������ʧ��\n");
		}
		else
		{
			//printf("TURE,�ȴ����տͻ���<socket=%d>���ӳɹ�\n",_cSock);
			//inet_ntoa(addCli.sin_addr)//���������ȡIP��ַ
			/*���µĿͻ��˷�����ͻ����������ٵ�cellServer*/
			addClienttoServer(new ClientScoket(_cSock));
		}
		return _cSock;
	}
	void addClienttoServer(ClientScoket* pClient)
	{
		//���ҿͻ��������ٵ�CellServer��Ϣ�������
		auto pMinServer = _cellServer[0];
		//������С�Ŀͻ��������ٵ�CellServer��Ϣ�߳�
		for (auto pClellServer:_cellServer)
		{
			if (pMinServer->getClientCount()>pClellServer->getClientCount())
			{
				pMinServer = pClellServer;
			}
		}
		pMinServer->addClient(pClient);
		OnNetJoin(pClient);
	}
	void Start(int nCellServerCount)
	{
		for (int i = 0; i < nCellServerCount; i++)
		{
			auto ser = new CellServer(_sock);
			_cellServer.push_back(ser);
			/*ע�������¼��Ľ��ܶ���*/
			ser->SetEventObj(this);
			/*������Ϣ�����߳�*/
			ser->Start();

		}
	}
	void Close()
	{
		if (_sock!=INVALID_SOCKET)
		{
#ifdef _WIN32
			//8 �ر��׽���
			closesocket(_sock);
			WSACleanup();
#else
			//8 �ر��׽���
			close(_sock);
#endif
		}

	}
	//����������Ϣ
	//int nCount = 0;
	bool OnRun()
	{
		if (IsRun())
		{
			time4msg();
			//������ socket
			fd_set fdRead;
			//�������
			FD_ZERO(&fdRead);// fd_set ����1024bit, ȫ����ʼ��Ϊ0
			//���뼯��
			FD_SET(_sock, &fdRead);//�������ļ�������fd��Ӧ�ı�־λ,����Ϊ1
			struct timeval _time;
			_time.tv_sec = 0;
			_time.tv_usec = 10;
			//nfds ��һ������ֵ ��ָfd_set���������е�������socket �ķ�Χ,����������,
	       //���������ļ����������ֵ+1��windows����ν ��linux��������
			int ret = select(_sock + 1, &fdRead, 0,0, &_time);
			if (ret < 0)
			{
				printf("Accept Select �������1\n");
				Close();
				return false;
			}
			// �ж�fd��Ӧ�ı�־λ������0����1, ����ֵ: fd��Ӧ�ı�־λ��ֵ, 0, ����0, 1->����1
			//��������
			//�ж�������(socket)�Ƿ��ڼ�����
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);// �������ļ�������fd��Ӧ�ı�־λ, ����Ϊ0
				Accept();
				return true;
			}
			return true;
		}
		return false;
	}
	/*�Ƿ�����*/
	bool IsRun() {
		return _sock != INVALID_SOCKET;
	}
	 /*���㲢���ÿ���յ���������Ϣ*/
	virtual void time4msg()
	{
		
		auto t1 = _time.getElapsedSecond();
		if (t1 >= 1.0)
		{
			
			
			printf("thread<%d> time=%lf socket<%d> recv<%d> RecvCount=%d\n", _cellServer.size(), t1, _sock, (int)(_recvCount/ t1),(int)(_MsgCount/t1));;
			_recvCount = 0;
			_time.update();
		}
		
	}
	//���͵���ָ����Socket
	/*int SendData(DataHeader*header,SOCKET clientSock)
	{
		if (IsRun()&&header!=nullptr)
		{
			return send(clientSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}*/
	
	/*�ͻ����뿪�¼�*/
	virtual void OnNetLeave(ClientScoket* pClient)
	{
		_ClientCount--;
	}
	virtual void OnNetMsg(DataHeader* header, ClientScoket*pClient) 
	{
		_recvCount++;
	}
	virtual void OnNetJoin(ClientScoket* pClient)
	{
		_ClientCount++;
	}
	virtual void OnNetRecv(ClientScoket* pClient){}

private:

};
#endif
