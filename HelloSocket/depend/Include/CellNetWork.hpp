#ifndef _CELL_NET_WORK_HPP_
#define _CELL_NET_WORK_HPP_

#include"Cell.hpp"

class CellNetWork
{
private:
	CellNetWork()
	{
#ifdef _WIN32
		//����Windows socket 2.x����
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif

#ifndef _WIN32
		//if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		//	return (1);
		//�����쳣�źţ�Ĭ������ᵼ�½�����ֹ
		signal(SIGPIPE, SIG_IGN);
#endif
	}

	~CellNetWork()
	{
#ifdef _WIN32
		//���Windows socket����
		WSACleanup();
#endif
	}
public:
	static void Init()
	{
		static  CellNetWork obj;
	}
};

#endif // !_CELL_NET_WORK_HPP_
