/*
  v1.0
*/
#pragma  once
#ifndef _CELL_TASK_H_
#define  _CELL_TASK_H_
#include <thread>
#include <mutex>
#include <list>
#include <functional>
//任务类型-基类
class CellTask
{
public:
	CellTask(){}
	//虚析构
   virtual~CellTask(){}
   //执行任务
   virtual void doTask() {}
};
class CellTaskServer
{
private:
	//任务数据
	std::list<CellTask*>_tasks;
	//任务数据缓冲区
	std::list<CellTask*>_taskBuf;
	//改变数据缓冲区的是需要枷锁
	std::mutex _mutex;
public:
	//添加任务
	void addTask(CellTask* task)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_taskBuf.push_back(task);
	}
	//启动工作服务线程
	void Start()
	{
		//线程
		std::thread _thread(std::mem_fn(&CellTaskServer::OnRun), this);
		//线程分离
		_thread.detach();
	}
	//工作函数
	void OnRun()
	{
		while (true)
		{
			//从缓冲区取出数据
			if (!_taskBuf.empty())
			{
				std::lock_guard<std::mutex>lock(_mutex);
				for (auto pTask:_taskBuf)
				{
					_tasks.push_back(pTask);
				}
				_taskBuf.clear();
			}
			//如果没有任务
			if (_taskBuf.empty())
			{
				std::chrono::microseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			//处理任务
			for ( auto pTask:_tasks)
			{
				pTask->doTask();
				delete pTask;
			}
			//清空任务
			_tasks.clear();
		}
	}
private:

};

#endif