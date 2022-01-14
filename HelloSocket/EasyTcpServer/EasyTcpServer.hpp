
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
	/*��Ϣ�������,�ڲ��ᴴ���߳�*/
	std::vector<CellServer*>_cellServer;
	/*ÿ����Ϣ��ʱ*/
	CELLTimestamp _time;

	CellThread _thread;
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
	SOCKET InitSocket()
	{
		CellNetWork::Init();
		if (_sock!=INVALID_SOCKET)
		{
			CellLog::Info("<Socket=%d>�ر�֮ǰ�ľ�����\n", _sock);
			Close();
		}
	//1.����Socket API ��������TC�����
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (_sock == INVALID_SOCKET)
		{
			CellLog::Info("ERROR,<socket=%d>����socketʧ��...\n", _sock);
		}
		else
		{
			CellLog::Info("TURE,<socket=%d>����socket�ɹ�.....\n", _sock);
		}
		return _sock;
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
			CellLog::Info("ERROR,�����ڽ��ܿͻ������ӵ�����˿�ʧ��...\n");
		}
		else
		{
			CellLog::Info("TURE,�����ڽ��ܿͻ������ӵ�����˿ڳɹ�.....\n");
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
			CellLog::Info("ERROR,��������˿�ʧ��...\n");
		}
		else
		{
			CellLog::Info("TURE,��������˿ڳɹ�.....\n");
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
			CellLog::Info("ERROR,�ȴ����տͻ�������ʧ��\n");
		}
		else
		{
			//inet_ntoa(addCli.sin_addr)//���������ȡIP��ַ
			/*���µĿͻ��˷�����ͻ����������ٵ�cellServer*/
			addClienttoServer(new CellClient(_cSock));
		}
		return _cSock;
	}
	void addClienttoServer(CellClient* pClient)
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
	}
	void Start(int nCellServerCount)
	{
		for (int i = 0; i < nCellServerCount; i++)
		{
			auto ser = new CellServer(i+1);
			_cellServer.push_back(ser);
			/*ע�������¼��Ľ��ܶ���*/
			ser->SetEventObj(this);
			/*������Ϣ�����߳�*/
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
			//8 �ر��׽���
			closesocket(_sock);
			WSACleanup();
#else
			//8 �ر��׽���
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
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
	
	virtual void OnNetJoin(CellClient* pClient)
	{
		_ClientCount++;
	}

	/*�ͻ����뿪�¼�*/
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
	//����������Ϣ
	//int nCount = 0;
	void OnRun(CellThread* pThread)
	{
		while(pThread->isRun())
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
			int ret = select(_sock + 1, &fdRead, 0, 0, &_time);
			if (ret < 0)
			{
				CellLog::Info("Accept Select �������1\n");
				_thread.Exit();
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
	/*���㲢���ÿ���յ���������Ϣ*/
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
