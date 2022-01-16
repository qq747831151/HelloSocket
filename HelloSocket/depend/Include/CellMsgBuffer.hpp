#ifndef _CELL_BUFFER_HPP_
#define _CELL_BUFFER_HPP_

#include"Cell.hpp"

class CellMsgBuffer
{
public:
	CellMsgBuffer(int nSize = 8192)
	{
		_nSize = nSize;
		_pBuff = new char[_nSize];
	}

	~CellMsgBuffer()
	{
		if (_pBuff)
		{
			delete[] _pBuff;
			_pBuff = nullptr;
		}
	}

	char* data()
	{
		return _pBuff;
	}

	bool push(const char* pData, int nLen)
	{
		////写入大量数据不一定要放到内存中
		////也可以存储到数据库或者磁盘等存储器中
		//if (_nLast + nLen > _nSize)
		//{
		//	//需要写入的数据大于可用空间
		//	int n = (_nLast + nLen) - _nSize;
		//	//拓展BUFF
		//	if (n < 8192)
		//		n = 8192;
		//	char* buff = new char[_nSize+n];
		//	memcpy(buff, _pBuff, _nLast);
		//	delete[] _pBuff;
		//	_pBuff = buff;
		//}

		if (_nLast + nLen <= _nSize)
		{
			//将要发送的数据 拷贝到发送缓冲区尾部
			memcpy(_pBuff + _nLast, pData, nLen);
			//计算数据尾部位置
			_nLast += nLen;

			if (_nLast == SEND_BUFF_SIZE)
			{
				++_fullCount;
			}

			return true;
		}
		else {
			++_fullCount;
		}

		return false;
	}

	void pop(int nLen)
	{
		int n = _nLast - nLen;
		if (n > 0)
		{
			memcpy(_pBuff, _pBuff + nLen, n);
		}
		/*剩余的数据*/
		_nLast = n;
		if (_fullCount > 0)
			--_fullCount;
	}
	/*立即将发送缓冲区的数据发送给客户端*/
	int write2socket(SOCKET sockfd)
	{
		int ret = 0;
		//缓冲区有数据
		if (_nLast > 0 && INVALID_SOCKET != sockfd)
		{
			//发送数据
			ret = send(sockfd, _pBuff, _nLast, 0);
			//数据尾部位置清零
			_nLast = 0;
			//
			_fullCount = 0;
		}
		return ret;
	}

	int read4socket(SOCKET sockfd)
	{
		//还有空间可以用
		if (_nSize - _nLast > 0)
		{
			//接收客户端数据
			char* szRecv = _pBuff + _nLast;
			//数据存到szRecv中  第三个参数可接收数据的最大长度
			int nLen = (int)recv(sockfd, szRecv, _nSize - _nLast, 0);//返回值是接收的长度  revcz在mac返回值是long 建议强转int
			//CELLLog::Info("nLen=%d\n", nLen);
			if (nLen <= 0)
			{
				return nLen;
			}
			//消息缓冲区的数据尾部位置后移
			_nLast += nLen;
			return nLen;
		}
		return 0;
	}

	bool hasMsg()
	{
		//解决粘包问题
		//判断消息缓冲区的数据长度大于消息头netmsg_DataHeader长度
		if (_nLast >= sizeof(DataHeader))
		{
			//这时就可以知道当前消息的长度
			DataHeader* header = (DataHeader*)_pBuff;

			/*这边是因为字节流这块 _pBuff里面的cmd和dataLength的数据对换了 现在还不知道哪里错了 暂时这么改*/
			if (header->cmd > header->dataLength)
			{
				DataHeader header1;
				header1.cmd = header->dataLength;
				header1.dataLength = header->cmd;
				header->cmd = header1.cmd;
				header->dataLength = header1.dataLength;
			}

			//判断消息缓冲区的数据长度大于消息长度
			return _nLast >= header->dataLength;
		}
		return false;
	}

	bool needWrite()
	{
		return _nLast > 0;
	}
private:
	//第二缓冲区 发送缓冲区
	char* _pBuff = nullptr;
	//可以用链表或队列来管理缓冲数据块
	//list<char*> _pBuffList;
	//缓冲区的数据尾部位置，已有数据长度
	int _nLast = 0;
	//缓冲区总的空间大小，字节长度
	int _nSize = 0;
	//缓冲区写满次数计数
	int _fullCount = 0;
};

#endif // !_CELL_BUFFER_HPP_
