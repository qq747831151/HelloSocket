#ifndef _CellClient_hpp_
#define _CellClient_hpp_

#include"Cell.hpp"

//客户端数据类型
class CellClient
{
public:
	CellClient(SOCKET sockfd = INVALID_SOCKET)
	{
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, RECV_BUFF_SIZE);
		_lastPos = 0;

		memset(_szSendBuf, 0, SEND_BUFF_SIZE);
		_lastSendPos = 0;
	}

	SOCKET Getsockfd()
	{
		return _sockfd;
	}

	char* GetmsgBuf()
	{
		return _szMsgBuf;
	}

	int GetLastPos()
	{
		return _lastPos;
	}
	void SetLastPos(int pos)
	{
		_lastPos = pos;
	}

	//发送数据
	int SendData(DataHeader* header)
	{
		int ret = SOCKET_ERROR;
		//要发送的数据长度
		int nSendLen = header->dataLength;
		//要发送的数据
		const char* pSendData = (const char*)header;

		while (true)
		{
			//定量 发送数据
			if (_lastSendPos + nSendLen >= SEND_BUFF_SIZE)
			{
				//计算可拷贝的数据长度
				int nCopyLen = SEND_BUFF_SIZE - _lastSendPos;
				//拷贝数据
				memcpy(_szSendBuf + _lastSendPos, pSendData, nCopyLen);
				//计算剩余数据位置
				pSendData += nCopyLen;
				//计算剩余数据长度
				nSendLen -= nCopyLen;
				//发送数据
				ret = send(_sockfd, _szSendBuf, SEND_BUFF_SIZE, 0);
				//数据尾部位置清零
				_lastSendPos = 0;
				//发送错误
				if (SOCKET_ERROR == ret)
				{
					return ret;
				}
			}
			else {
				//将要发送的数据 拷贝到发送缓冲区尾部
				memcpy(_szSendBuf + _lastSendPos, pSendData, nSendLen);
				//计算数据尾部位置
				_lastSendPos += nSendLen;
				break;
			}
		}
		return ret;
	}

private:
	// socket fd_set  file desc set
	SOCKET _sockfd;
	//第二缓冲区 消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE];
	//消息缓冲区的数据尾部位置
	int _lastPos;

	//第二缓冲区 发送缓冲区
	char _szSendBuf[SEND_BUFF_SIZE];
	//发送缓冲区的数据尾部位置
	int _lastSendPos;
};

#endif // !_CellClient_hpp_



