#ifndef  Cell_Thread_hpp
#define  Cell_Thread_hpp
#include "CellSemaphore.hpp"
#include<functional>
class CellThread
{
private: 
 typedef	std::function<void(CellThread*)> EventCall;
public:
	/*启动线程*/
	void Start(
		EventCall onCreate=nullptr,
		EventCall onDestory=nullptr,
		EventCall onRun=nullptr
		     )
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (!_isRun)
		{
			_isRun = true;
			if (onCreate)
				_onCreate = onCreate;
			if (onDestory)
				_onDestory = onDestory;
			if (onRun)
				_onRun = onRun;
			//线程
			std::thread t(std::mem_fn(&CellThread::OnWork), this);
			t.detach();
		}
	}
	/*关闭线程*/
	void Close()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;
			_sem.wait();
		}
	}
	/*在工作函数中退出
	  不需要使用信号量来阻塞等待
	  如果使用会阻塞住*/
	void  Exit()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;

		}
	}
	/*线程是否运行状态*/
	bool isRun()
	{
		return _isRun;
	}
protected:
	/*线程的运行时的工作函数*/
	void OnWork()
	{
		if (_onCreate)
			_onCreate(this);
		if (_onRun)
			_onRun(this);
		if (_onDestory)
			_onDestory(this);
		_sem.wakeup();
	}
		
private:
	/**/
	EventCall _onCreate;
	EventCall _onRun;
	EventCall _onDestory;
	//不同线程中改变数据
	std::mutex _mutex;
	/*控制线程终止 ,退出*/
	CellSemaphore _sem;
	/*线程是否启动运行中*/
	bool _isRun = false;

};
#endif