#define WIN32_LEAN_AND_MEAN

#define _WINSOCK_DEPRECATED_NO_WARNINGS //Ӱ��inet_addr
#include <windows.h>
#include <WINSock2.h>
#include <stdio.h>

/*Ϊ�˿���������ƽ̨Ҳ����ʹ�� �Ҽ���Ŀ���� ѡ�������� ���������� ��ws2_32.lib ��ӽ�ȥ���� �����Ͳ���Ҫ ������Щ */
#pragma  comment(lib,"ws2_32.lib")
enum CMD
{
	CMD_LOGIN,
	CMD_LOGINRESULT,
	CMD_LOGINOUT,
	CMD_LOGINOUTRESULT,
	CMD_ERROR,

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
	while (true)
	{
		//3.������������
		char recvBuf[128] = "";
		scanf("%s", recvBuf);
		//4.������������
		if (strcmp(recvBuf,"exit")==0)
		{
			printf("�յ�exit����,�������");
			break;
		}
		else if(0==strcmp(recvBuf,"login"))
		{
			//5.���������������
			Login login;
			strcpy(login.userName, "sfl");
			strcpy(login.passWord, "123456987.");

			send(sock, (const char*)&login, sizeof(Login), 0);

			//6.���շ���˷��ص�����
			LoginResult loginRet = {};
			recv(sock, (char*)&loginRet, sizeof(loginRet), 0);
			printf("LoginResult=%d\n", loginRet.result);
		}
		else if (0 == strcmp(recvBuf, "loginout"))
		{
			//5.���������������
			LoginOut loginout;
			strcpy(loginout.userName, "sfl");

			send(sock, (const char*)&loginout, sizeof(LoginOut), 0);
			//6.���շ���˷��ص�����
			LoginOutResult loginOutRet;
			recv(sock, (char*)&loginOutRet, sizeof(loginOutRet), 0);
			printf("LoginOutResult=%d\n", loginOutRet.result);
		}
		else
		{
			printf("�յ���֧�ֵ�����,����������");
		}
	}
	//7.�ر��׽���
	closesocket(sock);
	WSACleanup();
	getchar();
	return 0;
}