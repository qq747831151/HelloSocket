
#ifndef _CellLog_Hpp_
#define  _CellLog_Hpp_
#include <stdio.h>
#include "Cell.hpp"
#include "CellTask.hpp"
#include <ctime>
/*日志系统*/
class CellLog
{
//Info
//Debug
//Warring
//Error
public:
	CellLog()
	{

		_taskServer.Start();
	}
	~CellLog()
	{
		_taskServer.Close();
		if (_logFile)
		{
			fclose(_logFile);
			_logFile = nullptr;
		}
	}
public:
	static CellLog& Instance() {
		static CellLog sLog;
		return sLog;
	}
	void setLogPath(const char* logPath, const char* mode)
	{
		if (_logFile)
		{
			Info("CellLog:setLogPath Fclose\n");
			fclose(_logFile);
			_logFile = nullptr;
		}

		_logFile = fopen(logPath, mode);
		if (_logFile)
		{
			Info("CellLog:setLogPath Sucess，<%s>\n", logPath);
		}
		else
		{
			Info("CellLog:setLogPath Failed，<%s>\n", logPath);
		}
	}

	static void Info(const char* pStr)
	{


		CellLog* pLog = &Instance();
		pLog->_taskServer.addTask([=]()
			{
				if (pLog->_logFile)
				{
					/*设置一个当前时间出来*/
					auto t = system_clock::now();
					auto tNow = system_clock::to_time_t(t);
					//fprintf(pLog->_logFile, "%s", ctime(&tNow));
					std::tm* now = std::gmtime(&tNow);
					fprintf(pLog->_logFile, "{%d-%d-%d  %d:%d:%d}", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

					fprintf(pLog->_logFile, "%s", pStr);
					fflush(pLog->_logFile);
				}
				printf("%s", pStr);
			});
	}
	template<typename ...Args>
	static void Info(const char* pFormat, Args...args)
	{
		CellLog* pLog = &Instance();
		pLog->_taskServer.addTask([=]()
			{
				if (pLog->_logFile)
				{
					/*设置一个当前时间出来*/
					auto t = system_clock::now();
					auto tNow = system_clock::to_time_t(t);
					//fprintf(pLog->_logFile, "%s", ctime(&tNow));
					std::tm* now = std::gmtime(&tNow);
					fprintf(pLog->_logFile, "{%d-%d-%d  %d:%d:%d}", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

					fprintf(pLog->_logFile, pFormat, args...);
					fflush(pLog->_logFile);
				}
				printf(pFormat, args...);
			});

	}
private:
	FILE* _logFile = nullptr;
	CellTaskServer _taskServer;

};


#endif // !_CellLog_Hpp_
