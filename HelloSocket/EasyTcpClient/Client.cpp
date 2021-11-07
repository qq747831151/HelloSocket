#define WIN32_LEAN_AND_MEAN

#define _WINSOCK_DEPRECATED_NO_WARNINGS //Ӱ��inet_addr
#include <windows.h>
#include <WINSock2.h>
#include <stdio.h>
#include <thread>

/*Ϊ�˿���������ƽ̨Ҳ����ʹ�� �Ҽ���Ŀ���� ѡ�������� ���������� ��ws2_32.lib ��ӽ�ȥ���� �����Ͳ���Ҫ ������Щ */
#pragma  comment(lib,"ws2_32.lib")
enum CMD
{
	CMD_LOGIN,
	CMD_LOGINRESULT,
	CMD_LOGINOUT,
	CMD_LOGINOUTRESULT,
	CMD_ERROR,
	CMD_NEW_USER_JOIN,

};
//��Ϣͷ
struct DataHeader
{
	short cmd;//����
	short dataLength;//���ݳ���
};
//��¼
struct Login:DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];
};
//��¼���
struct LoginResult:DataHeader
{
	LoginResult()
	{
		cmd = CMD_LOGINRESULT;
		dataLength = sizeof(LoginResult);
	}
	int result=1;
};
//�ǳ�
struct LoginOut:DataHeader
{
	LoginOut()
	{
		cmd = CMD_LOGINOUT;
		dataLength = sizeof(LoginOut);

	}
	char userName[32];

};
//��¼���
struct LoginOutResult:DataHeader
{
	LoginOutResult()
	{
		cmd = CMD_LOGINOUTRESULT;
		dataLength = sizeof(LoginOutResult);
	}
	int result=1;
};

//���û�����
struct LoginNewUser :DataHeader
{
	LoginNewUser()
	{
		cmd = CMD_NEW_USER_JOIN;
		dataLength = sizeof(LoginNewUser);
		sock = 0;
	}
	int sock;
};
int Processor(SOCKET clientSock)
{
	//������
	char szRecv[1024] = {};

	//5.���ܷ���˷�����������
	//���ݴ浽szRecv��     �����������ǿɽ������ݵ���󳤶�
	int nlen = recv(clientSock, szRecv, sizeof(DataHeader), 0);//����ֵ�ǽ��յĳ���
	DataHeader* header = (DataHeader*)szRecv;
	if (nlen <= 0)
	{
		printf("�ͻ���<socket=%d>�˳�,�������\n", clientSock);
		return -1;
	}
	switch (header->cmd)
	{
	case  CMD_LOGINRESULT:
	{
		recv(clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginResult* loginResult = (LoginResult*)szRecv;
		printf("�յ���������Ϣ�� CMD_LOGINRESUL ���ݳ���:%d\n", loginResult->dataLength);
	}
	break;
	case  CMD_LOGINOUTRESULT:
	{
		recv(clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginOutResult* loginoutResult = (LoginOutResult*)szRecv;
		printf("�յ���������Ϣ�� CMD_LOGINOUTRESULT ���ݳ���:%d\n", loginoutResult->dataLength);
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		recv(clientSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginNewUser* loginnewUser = (LoginNewUser*)szRecv;
		printf("�յ���������Ϣ�� CMD_NEW_USER_JOIN ���ݳ���:%d\n", loginnewUser->dataLength);
	}
	break;
	}
}
bool g_bExit = true;//�߳��˳�
void cmdThread(SOCKET sock)
{
	while (true)
	{
		char szBuf[256];
		scanf("%s", szBuf);
		if (0 == strcmp(szBuf, "exit"))
		{
			printf("�˳�cmdThread\n");
			g_bExit = false;
			break;
		}
		else if (0 == strcmp(szBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "sfl");
			strcpy(login.passWord, "123");
			send(sock, (char*)&login, sizeof(login), 0);
		}
		else if (0 == strcmp(szBuf, "loginout"))
		{
			LoginOut loginOut;
			strcpy(loginOut.userName, "sfl");
			send(sock, (char*)&loginOut, sizeof(loginOut), 0);
		}
		else
		{
			printf("��֧������");
		}
	}
}
int main()
{
	/*����socket���绷�� 2.x����*/
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	/*��д*/
	//--��Socket API�������׵�TCP�ͻ���
	//1.����һ��socket�׽��� 
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock==INVALID_SOCKET)
	{
		printf("ERROR,�����׽���ʧ��\n");
	}
	else
	{
		printf("TRUE,�����׽��ֳɹ�\n");
	}
	//2.���ӷ����� connect
	sockaddr_in _sin;
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret=connect(sock, (struct sockaddr*)&_sin, sizeof(_sin));
	if (ret == INVALID_SOCKET)
	{
		printf("ERROR,���ӷ�����connectʧ��........\n");
	}
	else
	{
		printf("TRUE,�����ӷ�����connect�ɹ�.......\n");
	}
	//�����߳�
	std::thread t1(cmdThread,sock);
	t1.detach();//�̷߳���
	while (g_bExit)
	{

		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(sock, &fdRead);
		struct timeval _time;
		_time.tv_sec = 0;
		_time.tv_usec = 0;
		int ret = select(sock + 1, &fdRead, NULL, NULL, &_time);
		if (ret<0)
		{
			printf("select �������1\n");
			break;;
		}
		if (FD_ISSET(sock,&fdRead))
		{
			FD_CLR(sock, &fdRead);
			if (-1==Processor(sock))
			{
				printf("select �������2\n");
					break;
			}
		}
	
	}
	//7.�ر��׽���
	closesocket(sock);
	WSACleanup();
	getchar();
	return 0;
}