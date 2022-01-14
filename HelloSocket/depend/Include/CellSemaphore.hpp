#ifndef CELLSEMAPHORE_HPP
#define  CELLSEMAPHORE_HPP
#include <chrono>
#include <thread>
#include <mutex>
//条件变量
#include <condition_variable>
//信号量
class CellSemaphore
{
public:
	/*阻塞当前线程*/
	void wait()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		if (--_wait < 0)
		{
			/*阻塞等待*/
			_cv.wait(lock, [this]()->bool
				{
					return _wakeup > 0;
				});
			--_wakeup;
		}

	}
	/*唤醒当前线程*/
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
	//阻塞等待-条件变量
	std::condition_variable _cv;
	/*缓冲队列的锁*/
	std::mutex _mutex;
	//等待计数
	int _wait = 0;
	//唤醒计数
	int _wakeup = 0;

};




#endif // !CELLSEMAPHORE_HPP

//虚假唤醒