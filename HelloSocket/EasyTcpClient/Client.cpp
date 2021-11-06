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
	CMD_LOGINOUT,
	CMD_ERROR,

};
//��Ϣͷ
struct DataHeader
{
	short cmd;//����
	short dataLength;//���ݳ���
};
//��¼
struct Login
{
	char userName[32];
	char passWord[32];
};
//��¼���
struct LoginResult
{
	int result;
};
//�ǳ�
struct LoginOut
{
	char userName[32];

};
//��¼���
struct LoginOutResult
{
	int result;
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
			Login login;
			strcpy(login.userName, "sfl");
			strcpy(login.passWord, "123456987.");
			DataHeader dh = { CMD_LOGIN,sizeof(login) };
			//5.���������������
			send(sock, (const char*)&dh, sizeof(DataHeader), 0);
			send(sock, (const char*)&login, sizeof(Login), 0);
			//6.���շ���˷��ص�����
			DataHeader retHeader;
			LoginResult loginRet;
			recv(sock, (char*)&retHeader, sizeof(retHeader), 0);
			recv(sock, (char*)&loginRet, sizeof(loginRet), 0);
			printf("LoginResult=%d\n", loginRet.result);
		}
		else if (0 == strcmp(recvBuf, "loginout"))
		{
			LoginOut loginout;
			strcpy(loginout.userName, "sfl");
			DataHeader dh = { CMD_LOGINOUT,sizeof(loginout) };
			//5.���������������
			send(sock, (const char*)&dh, sizeof(DataHeader), 0);
			send(sock, (const char*)&loginout, sizeof(LoginOut), 0);
			//6.���շ���˷��ص�����
			DataHeader retHeader;
			LoginOutResult loginOutRet;
			recv(sock, (char*)&retHeader, sizeof(retHeader), 0);
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