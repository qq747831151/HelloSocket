#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>
/*Ϊ�˿���������ƽ̨Ҳ����ʹ�� �Ҽ���Ŀ���� ѡ�������� ���������� ��ws2_32.lib ��ӽ�ȥ���� �����Ͳ���Ҫ ������Щ */
//#pragma comment(lib,"ws2_32.lib")
int  main()
{
	/*����socket���绷�� 2.x����*/
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	/*��д*/
	WSACleanup();
	return 0;
}