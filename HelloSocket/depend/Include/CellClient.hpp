#ifndef _CellClient_hpp_
#define _CellClient_hpp_

#include"Cell.hpp"
#include "CellMsgBuffer.hpp"
#include "CellLog.hpp"

//客户端心跳检测死亡计时时间 60000毫秒=60秒钟
#define  CLIENT_HEARY_DEAD_TIME 60000

//在间隔指定时间内把发送缓冲区内缓存的数据发送给客户端
#define  CLIENT_SEND_BUFF_TIME 200

//客户端数据类型
class CellClient
{
public:
	int _ID = -1;
	int _serverID = -1;
public:
	CellClient(SOCKET sockfd = INVALID_SOCKET)
	{
		static int n = 1;
		_ID = n++;
		_sockfd = sockfd;
		resetDtHeart();
		resetDtSend();
	}
	~CellClient()
	{
		CellLog::Info("s=%d CellClient%d.~CellClient\n", _serverID, _ID);
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
	int RecvData()
	{
		return _recvBuff.read4Socket(_sockfd);
	}
	bool hasMsg()
	{
		return _recvBuff.hasMsg();
     }
	DataHeader* front_msg()
	{
		return (DataHeader*)_recvBuff.data();
	}
	void Pop_front_msg()
	{
		if (hasMsg())
		{
			_recvBuff.pop(front_msg()->dataLength);
		}
	}
	//立即发送数据 暂时用不到
	void SendData02(DataHeader* header)
	{
		SendData(header);
		SendDataReal();
	}

	//立即将发送缓冲区的数据发送给客户端
	int SendDataReal()
	{
		resetDtSend();
		return _sendBuff.Write2Socket(_sockfd);
	}

	/*缓冲区的控制根据业务需求的差异而调整*/
	//发送数据
	int SendData(DataHeader* header)
	{
		int ret = SOCKET_ERROR;
		//要发送的数据长度
		int nSendLen = header->dataLength;
		//要发送的数据
		const char* pSendData = (const char*)header;
		if (_sendBuff.push(pSendData,nSendLen))
		{
			return header->dataLength;
		}
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
			CellLog::Info("checkHeart dead=%d,time=%d\n", _sockfd, _dtHeart);
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
			//CellLog::Info("checkSend dead=%d,time=%d\n", _sockfd, _dtSend);
			//立即将发送缓冲区的数据发送出去
			SendDataReal();
			//重置发送计时
			resetDtSend();
			return true;
		}
		return false;

	}
private:
	// socket fd_set  file desc set
	SOCKET _sockfd;
	//第二缓冲区 消息缓冲区
	CellMsgBuffer _recvBuff;


	//第二缓冲区 发送缓冲区
	CellMsgBuffer _sendBuff;


	//心跳死亡计时
	time_t _dtHeart;

	//上次发送消息的时间
	time_t _dtSend;

	/*发送缓冲区遇到写满情况*/
	int _sendBuffFullCount = 0;
};


#endif // !_CellClient_hpp_



