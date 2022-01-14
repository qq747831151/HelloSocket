#ifndef _CellClient_hpp_
#define _CellClient_hpp_

#include"Cell.hpp"
#include "CellMsgBuffer.hpp"
#include "CellLog.hpp"

//�ͻ����������������ʱʱ�� 60000����=60����
#define  CLIENT_HEARY_DEAD_TIME 60000

//�ڼ��ָ��ʱ���ڰѷ��ͻ������ڻ�������ݷ��͸��ͻ���
#define  CLIENT_SEND_BUFF_TIME 200

//�ͻ�����������
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
	//������������ ��ʱ�ò���
	void SendData02(DataHeader* header)
	{
		SendData(header);
		SendDataReal();
	}

	//���������ͻ����������ݷ��͸��ͻ���
	int SendDataReal()
	{
		resetDtSend();
		return _sendBuff.Write2Socket(_sockfd);
	}

	/*�������Ŀ��Ƹ���ҵ������Ĳ��������*/
	//��������
	int SendData(DataHeader* header)
	{
		int ret = SOCKET_ERROR;
		//Ҫ���͵����ݳ���
		int nSendLen = header->dataLength;
		//Ҫ���͵�����
		const char* pSendData = (const char*)header;
		if (_sendBuff.push(pSendData,nSendLen))
		{
			return header->dataLength;
		}
	}
	/*�������������Ϊ0*/
	void resetDtHeart()
	{
		_dtHeart = 0;
	}
	//���ϴη�����Ϣ��ʱ������Ϊ0
	void resetDtSend()
	{
		_dtSend = 0;
	}
	//�������
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
	//��ʱ������Ϣ���
	bool checkSend(time_t dt)
	{
		_dtSend += dt;
		if (_dtSend>=CLIENT_SEND_BUFF_TIME)
		{
			//CellLog::Info("checkSend dead=%d,time=%d\n", _sockfd, _dtSend);
			//���������ͻ����������ݷ��ͳ�ȥ
			SendDataReal();
			//���÷��ͼ�ʱ
			resetDtSend();
			return true;
		}
		return false;

	}
private:
	// socket fd_set  file desc set
	SOCKET _sockfd;
	//�ڶ������� ��Ϣ������
	CellMsgBuffer _recvBuff;


	//�ڶ������� ���ͻ�����
	CellMsgBuffer _sendBuff;


	//����������ʱ
	time_t _dtHeart;

	//�ϴη�����Ϣ��ʱ��
	time_t _dtSend;

	/*���ͻ���������д�����*/
	int _sendBuffFullCount = 0;
};


#endif // !_CellClient_hpp_



