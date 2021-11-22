#ifndef _EasyTcpClient_Hpp
#define  _EasyTcpClient_Hpp


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
/*Ϊ�˿���������ƽ̨Ҳ����ʹ�� �Ҽ���Ŀ���� ѡ�������� ���������� ��ws2_32.lib ��ӽ�ȥ���� �����Ͳ���Ҫ ������Щ */
#pragma  comment(lib,"ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS //Ӱ��inet_addr
#include <windows.h>
#include <WINSock2.h>

#else

#include<unistd.h> //uni std
#include<arpa/inet.h>
#include<string.h>
#include<sys/select.h>
#include<pthread.h>

#define  SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR  (-1)
#endif



#include <stdio.h>
#include <thread>
#include "MessAgeHeader.hpp"


class EasyTcpClient
{
	bool _isConnect;
	SOCKET _sock;
public:
	EasyTcpClient() 
	{
		_sock = INVALID_SOCKET;//����Ϊ��
		_isConnect = false;
	}
	//����������
	virtual ~EasyTcpClient() 
	{
		Close();
	}
	void InitSocket()
	{
#ifdef _WIN32
		/*����socket���绷�� 2.x����*/
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		/*��д*/
	//--��Socket API�������׵�TCP�ͻ���
	//1.����һ��socket�׽��� 
		if (_sock!=INVALID_SOCKET)
		{
			printf("<socket=%d>�ر�֮ǰ�ľ�����\n", _sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (_sock == INVALID_SOCKET)
		{
			printf("ERROR,<Socket=%d>�����׽���ʧ��\n",_sock);
		}
		else
		{
			//printf("TRUE,<Socket=%d>�����׽��ֳɹ�\n",_sock);
		}
	}
	int Connect(const char *ip,unsigned short port )
	{
		//2.���ӷ����� connect
		sockaddr_in _sin;
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);//��������ӷ���˵�IP��ַ
#else
		inet_pton(AF_INET, "192.168.17.1", &_sin.sin_addr.s_addr);
#endif
		int ret = connect(_sock, (struct sockaddr*)&_sin, sizeof(_sin));
		if (ret == INVALID_SOCKET)
		{
			printf("ERROR,���ӷ�����connectʧ��........\n");
		}
		else
		{
			_isConnect = true;
			//printf("TRUE,�����ӷ�����connect�ɹ�.......\n");
		}
		return ret;
	}
	void Close()
	{
		if (_sock != INVALID_SOCKET) 
		{
#ifdef _WIN32
			//7.�ر��׽���
			closesocket(_sock);
			WSACleanup();
#else
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		}
		_isConnect = false;

	}
	//��ѯ������Ϣ
	int _nCount = 0;
	bool OnRun()
	{
		if (IsRun())
		{
			fd_set fdRead;
			FD_ZERO(&fdRead);
			FD_SET(_sock, &fdRead);
			struct timeval _time;
			_time.tv_sec = 0;
			_time.tv_usec = 0;
			int ret = select(_sock + 1, &fdRead, 0, 0, &_time);
			if (ret < 0)
			{
				printf("select �������1\n");
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				if (-1 == RecvData(_sock))
				{
					printf("select �������2\n");
					Close();
					return false;
				}
			}
			return true;
		}
		return false;

	}
	/*�Ƿ�����*/
	bool IsRun() {
		return _sock != INVALID_SOCKET&&_isConnect;
	}
#ifndef RECV_BUFF_SIZE
	//������������С��Ԫ��С 
#define RECV_BUFF_SIZE 10240
#endif 
	//���ջ�����
	char szRecv[RECV_BUFF_SIZE] = {};
	//�ڶ������� ��Ϣ������
	char szMsg[RECV_BUFF_SIZE * 5] = {};
	//��Ϣ���������β��λ��
	int lastPos = 0;
	/*��������  ����ճ�� ���*/
	int RecvData(SOCKET clientSock)
	{
		//5.���ܷ���˷�����������
		//���ݴ浽szRecv��     �����������ǿɽ������ݵ���󳤶�
		int nlen = (int)recv(clientSock, szRecv, RECV_BUFF_SIZE, 0);//����ֵ�ǽ��յĳ���  MAC�޸ĵĵط�
		if (nlen <= 0)
		{
			printf("�ͻ���<socket=%d>�˳�,�������\n", clientSock);
			return -1;
		}
		//����ȡ�������ݿ�������Ϣ������
		memcpy(szMsg+lastPos, szRecv, nlen);
		//��Ϣ������������β��λ�ú���
		lastPos +=nlen;

		//���ճ������
		//�ж���Ϣ�����������ݳ����Ƿ������ϢͷDataHeader ����
		while (lastPos>=sizeof(DataHeader))
		{
			//��ʱ�Ϳ���֪����ǰ��Ϣ����
			DataHeader* header = (DataHeader*)szMsg;
			//�ж���Ϣ��������ݳ��ȴ�����Ϣ�ĳ���
			if (lastPos>=header->dataLength)
			{
				//ʣ��δ�������Ϣ���������ݵĳ���
				int nSize = lastPos - header->dataLength;
				//����������Ϣ
				OnNetMsg(header);
				//����Ϣ������ʣ��δ��������ǰ��
				memcpy(szMsg, szMsg + header->dataLength, nSize);
				//��Ϣ������������β��λ��ǰ��
				lastPos = nSize;
			}
			else
			{
				//��Ϣ������ʣ������ ����һ����������Ϣ
				break;
			}
		}
		return 0;

	}
	//��Ӧ������Ϣ
	 virtual void OnNetMsg(DataHeader*header)
	{
		switch (header->cmd)
		{
		case  CMD_LOGINRESULT:
		{
			LoginResult* loginResult = (LoginResult*)header;
	//	printf("�յ���������Ϣ�� CMD_LOGINRESUL ���ݳ���:%d\n", loginResult->dataLength);
		}
		break;
		case  CMD_LOGINOUTRESULT:
		{
			LoginOutResult* loginoutResult = (LoginOutResult*)header;
			//printf("�յ���������Ϣ�� CMD_LOGINOUTRESULT ���ݳ���:%d\n", loginoutResult->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			LoginNewUser* loginnewUser = (LoginNewUser*)header;
			//printf("�յ���������Ϣ�� CMD_NEW_USER_JOIN ���ݳ���:%d\n", loginnewUser->dataLength);
		}
		break;
		case CMD_ERROR: {

			printf("<socket=%d>�յ��������Ϣ CMD_ERROR, ���ݳ��ȣ�%d \n", _sock,header->dataLength);
		}
					  break;
		default:
			//δ֪��Ϣ
			printf("<socket=%d>�յ�δ������Ϣ ���ݳ��ȣ�%d \n", _sock, header->dataLength);
			break;
		}
		
	}
	int SendData(DataHeader*header,const int nlen)
	{
		int ret = SOCKET_ERROR;
		if (IsRun()&&header)
		{
			
			ret=send(_sock, (const char*)header, nlen, 0);
			if (ret==SOCKET_ERROR)
			{
				Close();
			}
		}
		return SOCKET_ERROR;
	}
private:

};
#endif 

