///****************************************************
//	文件：MemoryMgr.hpp
//	作者：苏福龙
//	邮箱: 747831151@qq.com
//	日期：2021/07/10 19:31
//	功能：内存池管理者
//*****************************************************/
#ifndef _MemoryMgr_hpp_
#define _MemoryMgr_hpp_
#include <stdlib.h>
#include <assert.h>
#include <mutex>
#ifdef _DEBUG
#include <stdio.h>

#define  xPrintf(...) printf(__VA_ARGS__)

#else
#define  xPrintf(...)

#endif

#define  MAX_MEMORY_SIZE 1024
class MemoryAlloc;
//内存块 最小单元
class MemoryBlock
{
public:
	//所属内存块(池)
	MemoryAlloc* pAlloc;
	//下一个位置
	MemoryBlock* pNext;
	//内存块编号
	int nID;
	//引用次数 判断是否被重复使用
	int nRef;
	//是否在内存池中
	bool bPool;
private:
	//预留
	char a1;
	char a2;
	char a3;

};
//const unsigned int a = sizeof(MemoryBlock);
//内存池
class MemoryAlloc
{
public:
	MemoryAlloc()
	{
		_pBuf = nullptr;
		_pHeader = nullptr;
		_nSize = 0;
		_nBlcokSize = 0;
		//xPrintf("MemoryAlloc\n");
	}
	~MemoryAlloc()
	{
		if (_pBuf)
		{
			free(_pBuf);
		}

	}
	//申请内存
	void* allocMem(size_t nsize)
	{
		std::lock_guard<std::mutex> lg(_m);
		if (!_pBuf)
		{
			initMemory();
		}
		MemoryBlock* pReturn = nullptr;
		if (_pHeader == nullptr)
		{
			pReturn = (MemoryBlock*)malloc(nsize + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
		}
		else
		{
			pReturn = _pHeader;
			_pHeader = _pHeader->pNext;
			assert(0 == pReturn->nRef);
			pReturn->nRef = 1;

		}
		//xPrintf("allocMem:%p  id=%d size=%d\n", pReturn, pReturn->nID, nsize);
		return ((char*)pReturn + sizeof(MemoryBlock));
	}
	//释放内存
	void freeMem(void* pMem) {
		char* pData = (char*)pMem;
		MemoryBlock* pBlock = (MemoryBlock*)(pData - sizeof(MemoryBlock));//获得实际的位置
		assert(1 == pBlock->nRef);
		if (pBlock->bPool)
		{
			std::lock_guard<std::mutex> lg(_m);
			if (--pBlock->nRef != 0)
			{
				return;
			}
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		}
		else
		{
			if (--pBlock->nRef != 0)
			{
				return;
			}
			free(pBlock);
		}
	}
	//初始化内存池
	void initMemory()
	{
		//xPrintf("initMemory:_nSzie=%d,_nBlockSzie=%d\n", _nSize, _nBlcokSize);
		//断言
		assert(_pBuf == nullptr);
		if (_pBuf)
		{
			return;
		}
		//计算内存池的大小
		size_t realSize = _nSize + sizeof(MemoryBlock);//真实的内存
		size_t sizebuf = realSize * _nBlcokSize;
		//向系统申请池内存
		_pBuf = (char*)malloc(sizebuf);

		//初始化内存池
		_pHeader = (MemoryBlock*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pAlloc = this;
		_pHeader->pNext = nullptr;
		//遍历初始化内存块
		MemoryBlock* pTemp1 = _pHeader;
		for (int i = 1; i < _nBlcokSize; i++)
		{
			MemoryBlock* pTemp2 = (MemoryBlock*)(_pBuf + (i * realSize));
			pTemp2->bPool = true;
			pTemp2->nID = i;
			pTemp2->nRef = 0;
			pTemp2->pAlloc = this;
			pTemp2->pNext = nullptr;
			pTemp1->pNext = pTemp2;
			pTemp1 = pTemp2;
		}
	}
protected:
	//内存池地址
	char* _pBuf;
	//头部内存单元
	MemoryBlock* _pHeader;
	//内存单元的大小 总的大小
	size_t _nSize;
	//内存单元的数量
	size_t _nBlcokSize;
	std::mutex _m;


};
//便于在声明成员变量时初始化MemoryAlloc的成员数据
template<size_t nBlock, size_t nSize>
class MemoryAllocctor :public MemoryAlloc
{
public:
	MemoryAllocctor() {
		const size_t n = sizeof(void*);
		_nSize = (nSize / n) * n + (nSize % n ? n : 0);
		_nBlcokSize = nBlock;
	}
};
//内存管理工具 管理多个内存池
class MemoryMgr
{
private:
	MemoryMgr()
	{
		init(0, 64, &_mem64);
		init(65, 128, &_mem128);
		init(129, 256, &_mem256);
		init(257, 512, &_mem512);
		init(513, 1024, &_mem1024);
		//xPrintf("MemoryMgr\n");
	}
	~MemoryMgr()
	{

	}
public:
	static MemoryMgr& Instance()
	{
		//单例模式
		static MemoryMgr mgr;
		return mgr;
	}
	//申请内存
	void* allocMem(size_t nsize)
	{
		if (nsize <= MAX_MEMORY_SIZE)
		{
			return	_szAlloc[nsize]->allocMem(nsize);
		}
		else
		{
			MemoryBlock* pReturn = (MemoryBlock*)malloc(nsize + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nID = -1;
			pReturn->nRef = 1;
			pReturn->pAlloc = nullptr;
			pReturn->pNext = nullptr;
			//xPrintf("allocMem:%llx  id=%d size=%d", pReturn, pReturn->nID, nsize);
			//xPrintf("allocMem:%p  id=%d size=%d\n", pReturn, pReturn->nID, nsize);
			return ((char*)pReturn + sizeof(MemoryBlock));
		}

	}
	//释放内存
	void freeMem(void* pMem) {

		char* pData = (char*)pMem;
		MemoryBlock* pBlock = (MemoryBlock*)(pData - sizeof(MemoryBlock));
		xPrintf("freeMem:%p  id=%d\n", pBlock, pBlock->nID);
		if (pBlock->bPool)
		{
			pBlock->pAlloc->freeMem(pMem);
		}
		else
		{
			if (--pBlock->nRef == 0)
				free(pBlock);
		}
	}
	//增加内存块的引用计数
	void addRef(void* pMem)
	{
		char* pData = (char*)pMem;
		MemoryBlock* pBlock = (MemoryBlock*)(pData - sizeof(MemoryBlock));
		++pBlock->nRef;
	}
private:
	//初始化内存池映射数组
	void init(int nBegin, int nEnd, MemoryAlloc* pMem)
	{
		for (int i = nBegin; i <= nEnd; i++)
		{
			_szAlloc[i] = pMem;
		}
	}
private:
	MemoryAllocctor<100000, 64> _mem64;
	MemoryAllocctor<100000, 128> _mem128;
	MemoryAllocctor<100000, 256> _mem256;
	MemoryAllocctor<100000, 512> _mem512;
	MemoryAllocctor<100000, 1024> _mem1024;
	MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE + 1];
};


#endif // !_MemoryMgr_hpp_