#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#include"Cell.hpp"
#include"CellClient.hpp"
#include"CellServer.hpp"
#include"INetEvent.hpp"
#include"CellNetWork.hpp"

#include<thread>
#include<mutex>
#include<atomic>

class EasyTcpServer : public INetEvent
{
private:
	//
	CellThread _thread;
	//��Ϣ��������ڲ��ᴴ���߳�
	std::vector<CellServer*> _cellServers;
	//ÿ����Ϣ��ʱ
	CELLTimestamp _tTime;
	//
	SOCKET _sock;
protected:
	//SOCKET recv����
	std::atomic_int _recvCount;
	//�յ���Ϣ����
	std::atomic_int _msgCount;
	//�ͻ��˼���
	std::atomic_int _clientCount;
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;//��Ч��ַ
		_recvCount = 0;
		_msgCount = 0;
		_clientCount = 0;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}
	//��ʼ��Socket
	SOCKET InitSocket()
	{
		CellNetWork::Init();
		if (INVALID_SOCKET != _sock)
		{
			CellLog::Info("warning, initSocket close old socket<%d>...\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			CellLog::Info("error, create socket failed...\n");
		}
		else {
			CellLog::Info("create socket<%d> success...\n", (int)_sock);
		}
		return _sock;
	}

	//��IP�Ͷ˿ں�
	int Bind(const char* ip, unsigned short port)
	{
		//if (INVALID_SOCKET == _sock)
		//{
		//	InitSocket();
		//}
		// 2 bind �����ڽ��ܿͻ������ӵ�����˿�
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);//host to net unsigned short

#ifdef _WIN32
		if (ip) {
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
#else
		if (ip) {
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else {
			_sin.sin_addr.s_addr = INADDR_ANY;
		}
#endif
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret)
		{
			CellLog::Info("error, bind port<%d> failed...\n", port);
		}
		else {
			CellLog::Info("bind port<%d> success...\n", port);
		}
		return ret;
	}

	//�����˿ں�
	int Listen(int n)
	{
		// 3 listen ��������˿�
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)
		{
			CellLog::Info("error, listen socket<%d> failed...\n", _sock);
		}
		else {
			CellLog::Info("listen port<%d> success...\n", _sock);
		}
		return ret;
	}

	//���ܿͻ�������
	SOCKET Accept()
	{
		// 4 accept �ȴ����ܿͻ�������
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
		if (INVALID_SOCKET == cSock)
		{
			CellLog::Info("error, accept INVALID_SOCKET...\n");
		}
		else
		{
			//���¿ͻ��˷�����ͻ��������ٵ�cellServer
			addClientToCELLServer(new CellClient(cSock));
			//��ȡIP��ַ inet_ntoa(clientAddr.sin_addr)
		}
		return cSock;
	}

	void addClientToCELLServer(CellClient* pClient)
	{
		//���ҿͻ��������ٵ�CELLServer��Ϣ�������
		auto pMinServer = _cellServers[0];
		//������С�Ŀͻ��������ٵ�CellServer��Ϣ�߳�
		for (auto pServer : _cellServers)
		{
			if (pMinServer->getClientCount() > pServer->getClientCount())
			{
				pMinServer = pServer;
			}
		}
		pMinServer->addClient(pClient);
	}

	void Start(int nCELLServer)
	{
		for (int n = 0; n < nCELLServer; n++)
		{
			auto ser = new CellServer(n + 1);
			_cellServers.push_back(ser);
			//ע�������¼����ܶ���
			ser->setEventObj(this);
			//������Ϣ�����߳�
			ser->Start();
		}
		_thread.Start(nullptr,
			[this](CellThread* pThread) {
				OnRun(pThread);
			});
	}
	//�ر�Socket
	void Close()
	{
		CellLog::Info("EasyTcpServer.Close begin\n");
		_thread.Close();
		if (_sock != INVALID_SOCKET)
		{
			for (auto s : _cellServers)
			{
				delete s;
			}
			_cellServers.clear();
			//�ر��׽���socket
#ifdef _WIN32
			closesocket(_sock);
#else
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		}
		CellLog::Info("EasyTcpServer.Close end\n");
	}

	//cellServer 4 ����̴߳��� ����ȫ
	//���ֻ����1��cellServer���ǰ�ȫ��
	virtual void OnNetJoin(CellClient* pClient)
	{
		_clientCount++;
		//CELLLog::Info("client<%d> join\n", pClient->sockfd());
	}
	//cellServer 4 ����̴߳��� ����ȫ
	//���ֻ����1��cellServer���ǰ�ȫ��
	virtual void OnNetLeave(CellClient* pClient)
	{
		_clientCount--;
		//CELLLog::Info("client<%d> leave\n", pClient->sockfd());
	}
	//cellServer 4 ����̴߳��� ����ȫ
	//���ֻ����1��cellServer���ǰ�ȫ��
	virtual void OnNetMsg(CellServer* pServer, CellClient* pClient, DataHeader* header)
	{
		_msgCount++;
	}

	virtual void OnNetRecv(CellClient* pClient)
	{
		_recvCount++;
		//CELLLog::Info("client<%d> leave\n", pClient->sockfd());
	}
private:
	//����������Ϣ
	void OnRun(CellThread* pThread)
	{
		while (pThread->isRun())
		{
			time4msg();
			//�������׽��� BSD socket
			fd_set fdRead;//��������socket�� ����
						  //������
			FD_ZERO(&fdRead);
			//����������socket�����뼯��
			FD_SET(_sock, &fdRead);//�������ļ�������fd��Ӧ�ı�־λ,����Ϊ1
			///nfds ��һ������ֵ ��ָfd_set����������������(socket)�ķ�Χ������������
			///���������ļ����������ֵ+1 ��Windows�������������д0
			timeval t = { 0, 1 };
			int ret = select(_sock + 1, &fdRead, 0, 0, &t); //
			if (ret < 0)
			{
				CellLog::Info("EasyTcpServer.OnRun select exit.\n");
				pThread->Exit();
				break;
			}
			// �ж�fd��Ӧ�ı�־λ������0����1, ����ֵ: fd��Ӧ�ı�־λ��ֵ, 0, ����0, 1->����1
			//��������
			//�ж�������(socket)�Ƿ��ڼ�����
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);// �������ļ�������fd��Ӧ�ı�־λ, ����Ϊ0
				Accept();
			}
		}
	}

	//���㲢���ÿ���յ���������Ϣ
	void time4msg()
	{
		auto t1 = _tTime.getElapsedSecond();
		if (t1 >= 1.0)
		{
			CellLog::Info("thread<%d>,time<%lf>,socket<%d>,clients<%d>,recv<%d>,msg<%d>\n", (int)_cellServers.size(), t1, _sock, (int)_clientCount, (int)(_recvCount / t1), (int)(_msgCount / t1));
			_recvCount = 0;
			_msgCount = 0;
			_tTime.update();
		}
	}
};

#endif // !_EasyTcpServer_hpp_
