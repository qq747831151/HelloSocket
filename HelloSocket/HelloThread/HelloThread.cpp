

#include <iostream>
#include <thread>
#include <mutex>//锁
#include <atomic>//原子
#include "CELLTimestamp.hpp"
#include <time.h>
using namespace std;
//原子操作 原子 分子
mutex m;
const int tCount = 4;
atomic_int sum = 0;
//void workFun(int index)
//{
//	for (int i = 0; i < 200000; i++)
//	{
//		//临界区域-开始
//		//m.lock();
//		sum++;
//		//m.unlock();//如果用到公共资源可以锁住 临界区域锁住
//
//	}
//	cout << index << "Hello ,Main  thread\n" << endl;
//}//抢占式
//int main()
//{
//	thread t[tCount];
//	for (int i = 0; i < tCount; i++)
//	{
//		t[i] = thread(workFun, i);
//	}
//	CELLTimestamp tTime;
//	for (int i = 0; i < tCount; i++)
//	{
//		t[i].join();
//		//t[i].detach();
//	}
//	cout << tTime.getElapsedTimeInMilliSec() << ",sum=" << sum << endl;
//	sum = 0;
//	tTime.update();
//	for (int n = 0; n < 80000000; n++)
//	{
//		sum++;
//	}
//	cout << tTime.getElapsedTimeInMilliSec() << ",sum=" << sum << endl;
//	cout << "Hello,main thread." << endl;
//    std::cout << "Hello World!\n";
//	return 0;
//}
