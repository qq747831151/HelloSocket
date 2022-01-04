#ifndef _CellClient_hpp_
#define _CellClient_hpp_

#include"Cell.hpp"


//客户端心跳检测死亡计时时间 60000毫秒=60秒钟
#define  CLIENT_HEARY_DEAD_TIME 60000

//在间隔指定时间内把发送缓冲区内缓存的数据发送给客户端
#define  CLIENT_SEND_BUFF_TIME 200

//客户端数据类型
class CellClient
{
public:
	CellClient(SOCKET sockfd = INVALID_SOCKET)
	{
		static int n = 1;
		_ID = n++;
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, RECV_BUFF_SIZE);
		_lastPos = 0;

		memset(_szSendBuf, 0, SEND_BUFF_SIZE);
		_lastSendPos = 0;
		resetDtHeart();
		resetDtSend();
	}
	~CellClient()
	{
		printf("s=%d CellClient%d.~CellClient\n", _serverID, _ID);
		if (_sockfd != INVALID_SOCKET)
		{
#ifdef _WIN32
			closesocket(_sockfd);
#else
			close(_sockfd);
#endif
			_sockfd = 0;
		}

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

	//立即将发送缓冲区的数据发送给客户端
	int SendDataReal()
	{
		int ret = SOCKET_ERROR;
		//缓冲区有数据
		if (_lastSendPos>0&&SOCKET_ERROR!=_sockfd)
		{
			//发送数据
			ret = send(_sockfd, _szSendBuf, _lastSendPos, 0);
			//数据尾部位置设置为0
			_lastSendPos = 0;
		}
		return ret;
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
				//重置发送计时
				resetDtSend();
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
	/*将心跳检测重置为0*/
	void resetDtHeart()
	{
		_dtHeart = 0;
	}
	//将上次发送消息的时间重置为0
	void resetDtSend()
	{
		_dtSend = 0;
	}
	//心跳检测
	bool checkHeart(time_t dt)
	{
		_dtHeart += dt;
		if (_dtHeart>=CLIENT_HEARY_DEAD_TIME)
		{
			printf("checkHeart dead=%d,time=%d\n", _sockfd, _dtHeart);
			return true;
		}
		return false;
	}
	//定时发送消息检测
	bool checkSend(time_t dt)
	{
		_dtSend += dt;
		if (_dtSend>=CLIENT_SEND_BUFF_TIME)
		{
			//立即将发送缓冲区的数据发送出去
			SendDataReal();
			//重置发送计时
			resetDtSend();
			return true;
		}
		return false;

	}
public:
	int _ID = -1;
	int _serverID = -1;
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

	//心跳死亡计时
	time_t _dtHeart;

	//上次发送消息的时间
	time_t _dtSend;
};

#endif // !_CellClient_hpp_



