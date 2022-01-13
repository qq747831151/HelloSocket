#ifndef _CELLBUFFER_HPP_
#define  _CELLBUFFER_HPP_
#include "Cell.hpp"
class CellMsgBuffer
{
public:
	CellMsgBuffer(int nSize=8192)
	{
		_nSize = nSize;
		_pBuff = new char[_nSize];
	}
	~CellMsgBuffer()
	{
		if (_pBuff)
		{
			delete[]_pBuff;
			_pBuff = nullptr;
		}
	}
	char* data()
	{
		return _pBuff;
	}
	/*入栈*/
	bool push(const char*pData,int nLen)
	{
		if (nLen+_nlast<=_nSize)
		{
			/*将要发送的数据 拷贝到发送缓冲区尾部*/
			memcpy(_pBuff+_nlast, pData, nLen);
			//计算数据尾部位置
			_nlast += nLen;
			//
			if (_nlast==SEND_BUFF_SIZE)
			{
				_BuffFullCount++;
			}
			return true;
		}
		else
		{
			_BuffFullCount++;
		}
		return false;
	}

	/*出栈*/
	void pop(int nLen)
	{
		int n = _nlast - nLen;
		if (n>0)
		{
			memcpy(_pBuff, _pBuff + nLen, n);
		}
		/*剩余数据*/
		_nlast -= n;
		if (_BuffFullCount>0)
		{
			--_BuffFullCount;
		}
	}
	/*立即将发送缓冲区的数据发送给客户端*/
	int Write2Socket(SOCKET sockfd)
	{
		int ret = 0;
		/*缓冲区有数据*/
		if (_nlast>0&&0!=sockfd)
		{
			//发送数据
			ret = send(sockfd, _pBuff, _nlast, 0);
			//数据尾部位置设置0
			_nlast = 0;
			//
			_BuffFullCount = 0;
		}
		return ret;
	}
	int read4Socket(SOCKET sockfd)
	{
		//还有空间可以用
		if (_nSize-_nlast>0)
		{
			//接收客户端数据
			char* szRecv = _pBuff + _nlast;
			//数据存到szRecv中  第三个参数可接收数据的最大长度
			int nlen = (int)recv(sockfd, szRecv, _nSize - _nlast, 0);//返回值是接收的长度  revcz在mac返回值是long 建议强转int
			if (nlen<=0)
			{
				return nlen;
			}
		//消息缓冲区的数据尾部尾部位置后移
			_nlast += nlen;
			return nlen;
		}
		return 0;
	}
	bool hasMsg()
	{
		//解决粘包问题
		//判断消息缓冲区的数据长度是否大于消息头DataHeader长度
		if (_nlast>=sizeof(DataHeader))
		{
			//这时就可以知道当前消息的长度
			DataHeader* header = (DataHeader*)_pBuff;
			//判断消息缓冲的数据长度大于消息长度
			if (_nlast>=header->dataLength)
			{
				return true;
			}
		}
		return false;
	}
private:
	//第二缓冲区 发送缓冲区
	char* _pBuff = nullptr;
	////消息缓冲的数据尾部位置,已有数据长度
	int _nlast = 0;;
	//缓冲区总的空间大小,字节长度
	int _nSize = 0;;
	//缓冲区写满次数计数
	int _BuffFullCount = 0;;
};

#endif