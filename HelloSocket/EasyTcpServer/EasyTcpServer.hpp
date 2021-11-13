#ifndef _EasyTcpServer_HPP
#define  _EasyTcpServer_HPP
#ifdef _WIN32

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

#include "MessAgeHeader.hpp"
class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<SOCKET> g_clients;
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;//��Ч��ַ
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
			//������¿ͻ��˼���,�����������еĿͻ��˷�����Ϣ
			LoginNewUser newUser;
			SendData2All(&newUser);
			printf("�¿ͻ��˼���:socket=%d IP=%s\n", (int)_cSock, inet_ntoa(addCli.sin_addr));
			g_clients.push_back(_cSock);
		}
		return _cSock;
	}
	void Close()
	{
#ifdef _WIN32
		for (int i = g_clients.size() - 1; i >= 0; i--)
		{
			closesocket(g_clients[i]);
		}
		//8 �ر��׽���
		closesocket(_sock);
		WSACleanup();
#else
		for (int i = g_clients.size() - 1; i >= 0; i--)
		{
			close(g_clients[i]);
		}
		//8 �ر��׽���
		close(_sock);
#endif
	}
	//����������Ϣ
	int nCount = 0;
	bool OnRun()
	{
		if (IsRun())
		{
			//������ socket
			fd_set fdRead;
			fd_set fdWrite;
			fd_set fdExp;
			//�������
			FD_ZERO(&fdRead);// fd_set ����1024bit, ȫ����ʼ��Ϊ0
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);
			//���뼯��
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);//�������ļ�������fd��Ӧ�ı�־λ,����Ϊ1
			SOCKET maxSock = _sock;
			for (int i = g_clients.size() - 1; i >= 0; i--)
			{
				FD_SET(g_clients[i], &fdRead);//���Է�ֻ�� ��������Ҳ���Է�
				if (g_clients[i] > maxSock)
				{
					maxSock = g_clients[i];
				}
			}
			struct timeval _time;
			_time.tv_sec = 1;
			_time.tv_usec = 0;
			//nfds ��һ������ֵ ��ָfd_set���������е�������socket �ķ�Χ,����������,
	//���������ļ����������ֵ+1��windows����ν ��linux��������
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &_time);
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
			}
			//ͨ��,�пͻ��˷������ݹ���
			for (int i = g_clients.size() - 1; i >= 0; i--)
			{
				if (FD_ISSET(g_clients[i], &fdRead))
				{
					if (-1 == RecvData(g_clients[i]))
					{
						std::vector<SOCKET>::iterator iter = g_clients.begin() + i;;//i��ɾ���ĵ�����
						if (iter != g_clients.end())
						{
							g_clients.erase(iter);//ɾ��
						}
					}
				}
			}
			return true;
		}
		return false;
	}
	/*�Ƿ�����*/
	bool IsRun() {
		return _sock != INVALID_SOCKET;
	}
	//������
	char szRecv[409600] = {};
	//�������� ����ճ�� ��ְ�
	int RecvData(SOCKET clientSock)
	{
		//5.���ܿͻ�����������
		//���ݴ浽szRecv��     �����������ǿɽ������ݵ���󳤶�
		int nlen = recv(clientSock, szRecv, 409600, 0);//����ֵ�ǽ��յĳ���  revcz��mac����ֵ��long ����ǿתint
		if (nlen <= 0)
		{
			printf("�ͻ���<socket=%d>�˳�,�������\n", clientSock);
			return -1;
		}
		LoginResult loginresult;
		SendData(&loginresult, clientSock);

		/*DataHeader* header = (DataHeader*)szRecv;
		recv(clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader),0);
		OnNetMsg(header,clientSock);*/
		return 0;
	}
	//��Ӧ������Ϣ
	virtual void OnNetMsg(DataHeader*header,SOCKET clientSock)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;
			printf("�յ�������:%d ���ݳ���%d userName:%s password:%s\n", login->cmd, login->dataLength, login->userName, login->passWord);
			//���� �ж��û��������Ƿ���ȷ
			LoginResult loginresult;
			send(clientSock, (char*)&loginresult, sizeof(LoginResult), 0);
		}
		break;
		case  CMD_LOGINOUT:
		{
		
			LoginOut* loginout = (LoginOut*)header;
			printf("�յ�������:%d ���ݳ���%d userName:%s\n", loginout->cmd, loginout->dataLength, loginout->userName);
			//���� �ж��û��������Ƿ���ȷ
			LoginOutResult loginOutresult;
			send(clientSock, (char*)&loginOutresult, sizeof(LoginOutResult), 0);
		}
		break;
		default:
			DataHeader dp = { CMD_ERROR,0 };
			send(clientSock, (char*)&dp, sizeof(DataHeader), 0);
			break;
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
				SendData(header, g_clients[n]);
			}

		}
	}

private:

};
#endif
