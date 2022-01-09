#ifndef _CELL_TASK_H_
#define _CELL_TASK_H_

#include<thread>
#include<mutex>
#include<list>
#include <functional>
#include "CellSemaphore.hpp"
#include "CellThread.hpp"

//ִ������ķ�������
class CellTaskServer
{
public:
	//����serverID
	int _serverID = -1;
private:
	typedef std::function<void()> CellTask;
private:
	//��������
	 std::list<CellTask> _tasks;
	//�������ݻ�����
	std::list<CellTask> _tasksBuf;
	//�ı����ݻ�����ʱ��Ҫ����
	std::mutex _mutex;
	//
	CellThread _thread;
public:
	//�������
	void addTask(CellTask task)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_tasksBuf.push_back(task);
	}
	//���������߳�
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
	//��������
	void OnRun(CellThread*pthread)
	{
		while (pthread->isRun())
		{
			//�ӻ�����ȡ������
			if (!_tasksBuf.empty())
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pTask : _tasksBuf)
				{
					_tasks.push_back(pTask);
				}
				_tasksBuf.clear();
			}
			//���û������
			if (_tasks.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			//��������
			for (auto pTask : _tasks)
			{
				pTask();
				
			}
			//�������
			_tasks.clear();
		}
		//����������е�����
		for (auto pTask : _tasksBuf)
		{
			pTask();
		}
		printf("CellTaskServer%d.OnRun\n", _serverID);

	}
};
#endif // !_CELL_TASK_H_