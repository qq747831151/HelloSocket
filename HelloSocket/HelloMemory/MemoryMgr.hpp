///****************************************************
//	�ļ���MemoryMgr.hpp
//	���ߣ��ո���
//	����: 747831151@qq.com
//	���ڣ�2021/07/10 19:31
//	���ܣ��ڴ�ع�����
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
//�ڴ�� ��С��Ԫ
class MemoryBlock
{
public:
	//�����ڴ��(��)
	MemoryAlloc* pAlloc;
	//��һ��λ��
	MemoryBlock* pNext;
	//�ڴ����
	int nID;
	//���ô��� �ж��Ƿ��ظ�ʹ��
	int nRef;
	//�Ƿ����ڴ����
	bool bPool;
private:
	//Ԥ��
	char a1;
	char a2;
	char a3;

};
//const unsigned int a = sizeof(MemoryBlock);
//�ڴ��
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
	//�����ڴ�
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
	//�ͷ��ڴ�
	void freeMem(void* pMem) {
		char* pData = (char*)pMem;
		MemoryBlock* pBlock = (MemoryBlock*)(pData - sizeof(MemoryBlock));//���ʵ�ʵ�λ��
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
	//��ʼ���ڴ��
	void initMemory()
	{
		//xPrintf("initMemory:_nSzie=%d,_nBlockSzie=%d\n", _nSize, _nBlcokSize);
		//����
		assert(_pBuf == nullptr);
		if (_pBuf)
		{
			return;
		}
		//�����ڴ�صĴ�С
		size_t realSize = _nSize + sizeof(MemoryBlock);//��ʵ���ڴ�
		size_t sizebuf = realSize * _nBlcokSize;
		//��ϵͳ������ڴ�
		_pBuf = (char*)malloc(sizebuf);

		//��ʼ���ڴ��
		_pHeader = (MemoryBlock*)_pBuf;
		_pHeader->bPool = true;
		_pHeader->nID = 0;
		_pHeader->nRef = 0;
		_pHeader->pAlloc = this;
		_pHeader->pNext = nullptr;
		//������ʼ���ڴ��
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
	//�ڴ�ص�ַ
	char* _pBuf;
	//ͷ���ڴ浥Ԫ
	MemoryBlock* _pHeader;
	//�ڴ浥Ԫ�Ĵ�С �ܵĴ�С
	size_t _nSize;
	//�ڴ浥Ԫ������
	size_t _nBlcokSize;
	std::mutex _m;


};
//������������Ա����ʱ��ʼ��MemoryAlloc�ĳ�Ա����
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
//�ڴ������ �������ڴ��
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
		//����ģʽ
		static MemoryMgr mgr;
		return mgr;
	}
	//�����ڴ�
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
	//�ͷ��ڴ�
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
	//�����ڴ������ü���
	void addRef(void* pMem)
	{
		char* pData = (char*)pMem;
		MemoryBlock* pBlock = (MemoryBlock*)(pData - sizeof(MemoryBlock));
		++pBlock->nRef;
	}
private:
	//��ʼ���ڴ��ӳ������
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