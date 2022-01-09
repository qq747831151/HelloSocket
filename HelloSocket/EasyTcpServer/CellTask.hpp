#ifndef _CELL_TASK_H_
#define _CELL_TASK_H_

#include<thread>
#include<mutex>
#include<list>
#include <functional>
#include "CellSemaphore.hpp"
#include "CellThread.hpp"

//执行任务的服务类型
class CellTaskServer
{
public:
	//所属serverID
	int _serverID = -1;
private:
	typedef std::function<void()> CellTask;
private:
	//任务数据
	 std::list<CellTask> _tasks;
	//任务数据缓冲区
	std::list<CellTask> _tasksBuf;
	//改变数据缓冲区时需要加锁
	std::mutex _mutex;
	//
	CellThread _thread;
public:
	//添加任务
	void addTask(CellTask task)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_tasksBuf.push_back(task);
	}
	//启动工作线程
	void Start()
	{
		_thread.Start(nullptr, [this](CellThread* pThread)
			{
				OnRun(pThread);
			});
	}
	void Close()
	{
		printf("CellTaskServer%d.Close1\n", _serverID);
		_thread.Close();
		printf("CellTaskServer%d.Close2\n", _serverID);
	}
protected:
	//工作函数
	void OnRun(CellThread*pthread)
	{
		while (pthread->isRun())
		{
			//从缓冲区取出数据
			if (!_tasksBuf.empty())
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pTask : _tasksBuf)
				{
					_tasks.push_back(pTask);
				}
				_tasksBuf.clear();
			}
			//如果没有任务
			if (_tasks.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			//处理任务
			for (auto pTask : _tasks)
			{
				pTask();
				
			}
			//清空任务
			_tasks.clear();
		}
		//处理缓冲队列中的任务
		for (auto pTask : _tasksBuf)
		{
			pTask();
		}
		printf("CellTaskServer%d.OnRun\n", _serverID);

	}
};
#endif // !_CELL_TASK_H_