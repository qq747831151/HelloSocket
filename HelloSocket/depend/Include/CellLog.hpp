#ifndef _CELL_LOG_HPP_
#define _CELL_LOG_HPP_

#include"Cell.hpp"
#include"CellTask.hpp"
#include<ctime>
class CellLog
{
	//Info
	//Debug
	//Warring
	//Error
private:
	CellLog()
	{
		_taskServer.Start();
	}

	~CellLog()
	{
		_taskServer.Close();
		if (_logFile)
		{
			Info("CELLLog fclose(_logFile)\n");
			fclose(_logFile);
			_logFile = nullptr;
		}
	}
public:
	static CellLog& Instance()
	{
		static  CellLog sLog;
		return sLog;
	}

	void setLogPath(const char* logPath, const char* mode)
	{
		if (_logFile)
		{
			Info("CELLLog::setLogPath _logFile != nullptr\n");
			fclose(_logFile);
			_logFile = nullptr;
		}


		_logFile = fopen(logPath, mode);
		if (_logFile)
		{
			Info("CELLLog::setLogPath success,<%s,%s>\n", logPath, mode);
		}
		else {
			Info("CELLLog::setLogPath failed,<%s,%s>\n", logPath, mode);
		}
	}

	static void Info(const char* pStr)
	{
		CellLog* pLog = &Instance();
		pLog->_taskServer.addTask([=]() {
			if (pLog->_logFile)
			{
				auto t = system_clock::now();
				auto tNow = system_clock::to_time_t(t);
				//fprintf(pLog->_logFile, "%s", ctime(&tNow));
				std::tm* now = std::gmtime(&tNow);
				fprintf(pLog->_logFile, "%s", "Info ");
				fprintf(pLog->_logFile, "[%d-%d-%d %d:%d:%d]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
				fprintf(pLog->_logFile, "%s", pStr);
				fflush(pLog->_logFile);
			}
			printf("%s", pStr);
			});
	}

	template<typename ...Args>
	static void Info(const char* pformat, Args ... args)
	{
		CellLog* pLog = &Instance();
		pLog->_taskServer.addTask([=]() {
			if (pLog->_logFile)
			{
				auto t = system_clock::now();
				auto tNow = system_clock::to_time_t(t);
				//fprintf(pLog->_logFile, "%s", ctime(&tNow));
				std::tm* now = std::gmtime(&tNow);
				fprintf(pLog->_logFile, "%s", "Info ");
				fprintf(pLog->_logFile, "[%d-%d-%d %d:%d:%d]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
				fprintf(pLog->_logFile, pformat, args...);
				fflush(pLog->_logFile);
			}
			printf(pformat, args...);
			});
	}
private:
	FILE* _logFile = nullptr;
	CellTaskServer _taskServer;
};

#endif // !_CELL_LOG_HPP_
