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
#include"mutex"
#ifndef RECV_BUFF_SIZE
//������������С��Ԫ��С 
#define RECV_BUFF_SIZE 10240
#endif 
#define _CellServer_THREAD_COUNT 4

/*�洢�ͻ��˵���Ϣ*/
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
	//�ڶ������� ��Ϣ������
	char szMsg[RECV_BUFF_SIZE * 10];
	//��Ϣ���������β����λ��
	int lastPos;
	SOCKET _sockfd;

};
class INetEvent
{
public:
	/*�ͻ����뿪�¼�*/
	virtual void OnLeave(ClientScoket* pClient) = 0;//���麯��
	/**/
	virtual void OnNetMsg(DataHeader* header, SOCKET _clientSock) = 0;//���麯��

private:

};


/*����ͻ�����Ϣ*/
class CellServer
{
private:
	std::mutex _mutex;
	SOCKET _sock;
	//��ʽ�ͻ��˶���
	std::vector<ClientScoket*>_clients;
	//����ͻ��˶���
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
	//�ر�Socket
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
			//���windows socket����
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
	/*�Ƿ�����*/
	bool IsRun() {
		return _sock != INVALID_SOCKET;
	}
	//����������Ϣ
	bool OnRun()
	{
		while (IsRun())
		{
			//������
			if (_clientsBuf.size()>0)
			{
				//�ӻ�����������ȡ���ͻ�����
				std::lock_guard<std::mutex> lock(_mutex);//��ס
				for (auto pClient : _clientsBuf)
				{
					_clients.push_back(pClient);
				}
				_clientsBuf.clear();
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
			SOCKET maxSock =_clients[0]->Getsockfd();
			for (int i = (int)_clients.size() - 1; i >= 0; i--)
			{
				FD_SET(_clients[i]->Getsockfd(), &fdRead);//���Է�ֻ�� ��������Ҳ���Է�
				if (_clients[i]->Getsockfd() > maxSock)
				{
					maxSock = _clients[i]->Getsockfd();
				}
			}
			///nfds ��һ������ֵ ��ָfd_set����������������(socket)�ķ�Χ������������
			///���������ļ����������ֵ+1 ��Windows�������������д0
			int ret = select(maxSock + 1, &fdRead, nullptr, nullptr, nullptr);
			if (ret < 0)
			{
				printf("select���������\n");
				Close();
				return false;
			}
			for (int i = (int)_clients.size() - 1; i >= 0; i--)
			{
				if (FD_ISSET(_clients[i]->Getsockfd(),&fdRead))
				{
					if (-1 == RecvData(_clients[i]))
					{
						std::vector<ClientScoket*>::iterator iter = _clients.begin() + i;;//i��ɾ���ĵ�����
						if (iter != _clients.end())
						{
							if (_pNetevt)
							{
								_pNetevt->OnLeave(_clients[i]);
							}
							delete _clients[i];
							_clients.erase(iter);//ɾ��
						}
					}
				}
			}
		}
	}
	//������
	char szRecv[RECV_BUFF_SIZE] = {};
	//�������� ����ճ�� ��ְ�
	int RecvData(ClientScoket* clientSock)
	{
		//5.���ܿͻ�����������
		//���ݴ浽szRecv��     �����������ǿɽ������ݵ���󳤶�
		int nlen = (int)recv(clientSock->Getsockfd(), szRecv, RECV_BUFF_SIZE, 0);//����ֵ�ǽ��յĳ���  revcz��mac����ֵ��long ����ǿתint
		if (nlen <= 0)
		{
			printf("�ͻ���<socket=%d>�˳�,�������\n", clientSock->Getsockfd());
			return -1;
		}

		//����ȡ������ݿ�������Ϣ������
		memcpy(clientSock->GetmsgBuf() + clientSock->GetLastPos(), szRecv, nlen);
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
				OnNetMsg(clientSock->Getsockfd(),header);
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
	virtual void OnNetMsg(SOCKET cSock, DataHeader* header)
	{
		_recvCount++;
		_pNetevt->OnNetMsg( header, cSock);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{

			Login* login = (Login*)header;
			//printf("�յ��ͻ���<Socket=%d>����CMD_LOGIN,���ݳ��ȣ�%d,userName=%s PassWord=%s\n", cSock, login->dataLength, login->userName, login->PassWord);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			//LoginResult ret;
			//SendData(cSock, &ret);
		}
		break;
		case CMD_LOGINOUT:
		{
			LoginOut* logout = (LoginOut*)header;
			//printf("�յ��ͻ���<Socket=%d>����CMD_LOGOUT,���ݳ��ȣ�%d,userName=%s \n", cSock, logout->dataLength, logout->userName);
			//�����ж��û������Ƿ���ȷ�Ĺ���
			//LogoutResult ret;
			//SendData(cSock, &ret);
		}
		break;
		default:
		{
			printf("<socket=%d>�յ�δ������Ϣ,���ݳ��ȣ�%d\n", cSock, header->dataLength);
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
		_sock = INVALID_SOCKET;//��Ч��ַ
		//_recvCount = 0;
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
			printf("TURE,�ȴ����տͻ���<socket=%d>���ӳɹ�\n",_cSock);
			//������¿ͻ��˼���,�����������еĿͻ��˷�����Ϣ
			//LoginNewUser newUser;
			//SendData2All(&newUser);
			//printf("�¿ͻ��˼���:socket=%d IP=%s\n", (int)_cSock, inet_ntoa(addCli.sin_addr));
			addClienttoServer(new ClientScoket(_cSock));
		}
		return _cSock;
	}
	void addClienttoServer(ClientScoket* pClient)
	{
		g_clients.push_back(pClient);
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
			//8 �ر��׽���
			closesocket(_sock);
			WSACleanup();
#else
			for (int i = g_clients.size() - 1; i >= 0; i--)
			{
				close(g_clients[i]->Getsockfd());
				delete g_clients[i];
			}
			//8 �ر��׽���
			close(_sock);
#endif
		}
		g_clients.clear();

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
			_time.tv_sec = 1;
			_time.tv_usec = 0;
			//nfds ��һ������ֵ ��ָfd_set���������е�������socket �ķ�Χ,����������,
	       //���������ļ����������ֵ+1��windows����ν ��linux��������
			int ret = select(_sock + 1, &fdRead, 0,0, &_time);
			if (ret < 0)
			{
				printf("�ͻ������˳�,�������\n");
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
	//��Ӧ������Ϣ
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
	//���͵���ָ����Socket
	int SendData(DataHeader*header,SOCKET clientSock)
	{
		if (IsRun()&&header!=nullptr)
		{
			return send(clientSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	//����Ⱥ������
	void SendData2All(DataHeader* header)
	{
		if (IsRun() && header != NULL)
		{
			//������¿ͻ��˼��� �����������еĿͻ��˷���
			for (int n = g_clients.size() - 1; n >= 0; n--)
			{
				SendData(header, g_clients[n]->Getsockfd());
			}

		}
	}
	/*�ͻ����뿪�¼�*/
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
