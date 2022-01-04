#ifndef CELLSEMAPHORE_HPP
#define  CELLSEMAPHORE_HPP
#include <chrono>
#include <thread>
#include <mutex>
//��������
#include <condition_variable>
//�ź���
class CellSemaphore
{
public:
	/*������ǰ�߳�*/
	void wait()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		if (--_wait < 0)
		{
			/*�����ȴ�*/
			_cv.wait(lock, [this]()->bool
				{
					return _wakeup > 0;
				});
			--_wakeup;
		}

	}
	/*���ѵ�ǰ�߳�*/
	void wakeup()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (++_wait <= 0)
		{
			++_wakeup;
			_cv.notify_one();
		}
		else
		{
			printf("CellSemaphore ERROR");
		}
	}


private:
	//�����ȴ�-��������
	std::condition_variable _cv;
	/*������е���*/
	std::mutex _mutex;
	//�ȴ�����
	int _wait = 0;
	//���Ѽ���
	int _wakeup = 0;

};




#endif // !CELLSEMAPHORE_HPP

//��ٻ���