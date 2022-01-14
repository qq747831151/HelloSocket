
#ifndef _CellNetWork_Hpp_
#include <WinSock2.h>
#define  _CellNetWork_Hpp_
class CellNetWork
{
private:
	CellNetWork()
	{
#ifdef _WIN32
		/*启动socket网络环境 2.x环境*/
		WORD ver = MAKEWORD(2, 2);//版本号
		WSADATA dat;
		WSAStartup(ver, &dat);//动态库需要写上那个lib
#endif

#ifndef _WIN32
		//忽略异常信号 ,默认情况会导致进程终止
		signal(SIGPIPE, SIG_IGN);
		/*if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		{
			return (1);
		}*/
#else
		//signal(SIGPIPE,SIG_IGN)；
#endif 
	}
	~CellNetWork()
	{
#ifdef _WIN32
		//清除windows socket环境
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