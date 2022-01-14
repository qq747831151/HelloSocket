
#ifndef _CellNetWork_Hpp_
#include <WinSock2.h>
#define  _CellNetWork_Hpp_
class CellNetWork
{
private:
	CellNetWork()
	{
#ifdef _WIN32
		/*����socket���绷�� 2.x����*/
		WORD ver = MAKEWORD(2, 2);//�汾��
		WSADATA dat;
		WSAStartup(ver, &dat);//��̬����Ҫд���Ǹ�lib
#endif

#ifndef _WIN32
		//�����쳣�ź� ,Ĭ������ᵼ�½�����ֹ
		signal(SIGPIPE, SIG_IGN);
		/*if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		{
			return (1);
		}*/
#else
		//signal(SIGPIPE,SIG_IGN)��
#endif 
	}
	~CellNetWork()
	{
#ifdef _WIN32
		//���windows socket����
		WSACleanup();
#endif 
	}

public:
	static void Init() {
		static CellNetWork obj;
		//return obj;
	}

};

#endif 