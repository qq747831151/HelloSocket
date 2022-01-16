#ifndef _CELL_TASK_H_
#define _CELL_TASK_H_

#include<thread>
#include<mutex>
#include<list>

#include<functional>

#include"CellThread.hpp"

//ִ������ķ�������
class CellTaskServer
{
public:
	//����serverid
	int serverId = -1;
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
		_thread.Start(nullptr, [this](CellThread* pThread) {
			OnRun(pThread);
			});
	}

	void Close()
	{
		///CELLLog::Info("CELLTaskServer%d.Close begin\n", serverId);
		_thread.Close();
		//CELLLog::Info("CELLTaskServer%d.Close end\n", serverId);
	}
protected:
	//��������
	void OnRun(CellThread* pThread)
	{
		while (pThread->isRun())
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
		//CELLLog::Info("CELLTaskServer%d.OnRun exit\n", serverId);
	}
};
#endif // !_CELL_TASK_H_
