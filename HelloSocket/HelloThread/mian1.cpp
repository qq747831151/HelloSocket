

#include <iostream>
#include <thread>
#include <mutex>//锁
#include <atomic>//原子
#include "CELLTimestamp.hpp"
#include <time.h>
using namespace std;
/*
void workFun()
{
	for (int i = 0; i < 5; i++)
	{
		
		cout << "Hello ,Other  thread" << endl;
	}
	
}*///抢占式
//int main()
//{
//	thread t(workFun);
//	t.detach();
//	for (int n = 0; n < 4; n++)
//		cout << "Hello,main thread." << endl;
//	while (true)
//	{
//
//	}
//	return 0;
//}
