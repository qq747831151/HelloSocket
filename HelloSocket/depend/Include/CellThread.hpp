#ifndef _CELL_THREAD_HPP_
#define _CELL_THREAD_HPP_

#include"CellSemaphore.hpp"

class CellThread
{
private:
	typedef std::function<void(CellThread*)> EventCall;
public:
	//�����߳�
	void Start(
		EventCall onCreate = nullptr,
		EventCall onRun = nullptr,
		EventCall onDestory = nullptr)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (!_isRun)
		{
			_isRun = true;

			if (onCreate)
				_onCreate = onCreate;
			if (onRun)
				_onRun = onRun;
			if (onDestory)
				_onDestory = onDestory;

			//�߳�
			std::thread t(std::mem_fn(&CellThread::OnWork), this);
			t.detach();
		}
	}

	//�ر��߳�
	void Close()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;
			_sem.wait();
		}
	}
	//�ڹ����������˳�
	//����Ҫʹ���ź����������ȴ�
	//���ʹ�û�����
	void Exit()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;
		}
	}

	//�߳��Ƿ���������״̬
	bool isRun()
	{
		return _isRun;
	}
protected:
	//�̵߳�����ʱ�Ĺ�������
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
	EventCall _onCreate;
	EventCall _onRun;
	EventCall _onDestory;
	//��ͬ�߳��иı�����ʱ��Ҫ����
	std::mutex _mutex;
	//�����̵߳���ֹ���˳�
	CellSemaphore _sem;
	//�߳��Ƿ�����������
	bool	_isRun = false;
};


#endif // !_CELL_THREAD_HPP_
