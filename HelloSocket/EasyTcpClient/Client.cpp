#define WIN32_LEAN_AND_MEAN

#define _WINSOCK_DEPRECATED_NO_WARNINGS //Ӱ��inet_addr
#include <windows.h>
#include <WINSock2.h>
#include <stdio.h>

/*Ϊ�˿���������ƽ̨Ҳ����ʹ�� �Ҽ���Ŀ���� ѡ�������� ���������� ��ws2_32.lib ��ӽ�ȥ���� �����Ͳ���Ҫ ������Щ */
#pragma  comment(lib,"ws2_32.lib")

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
		printf("ERROR,�����׽���ʧ��");
	}
	else
	{
		printf("TRUE,�����׽��ֳɹ�");
	}
	//2.���ӷ����� connect
	sockaddr_in _sin;
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret=connect(sock, (struct sockaddr*)&_sin, sizeof(_sin));
	if (ret == INVALID_SOCKET)
	{
		printf("ERROR,���ӷ�����connectʧ��........");
	}
	else
	{
		printf("TRUE,�����ӷ�����connect�ɹ�.......");
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
		else
		{
			//5.���������������
			send(sock, recvBuf, strlen(recvBuf) + 1, 0);
		}
		//6.���շ�������Ϣ
		char msgBuf[128] = "";
		int nlen = recv(sock, msgBuf, sizeof(msgBuf), 0);
		printf("%d\n", nlen);
		if (nlen>0)
		{
			printf("���յ�������Ϊ%s\n", msgBuf);
		}
	}
	//7.�ر��׽���
	closesocket(sock);
	WSACleanup();
	getchar();
	return 0;
}