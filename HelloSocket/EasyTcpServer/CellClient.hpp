#ifndef _CellClient_hpp_
#define _CellClient_hpp_

#include"Cell.hpp"


//�ͻ����������������ʱʱ�� 60000����=60����
#define  CLIENT_HEARY_DEAD_TIME 60000

//�ڼ��ָ��ʱ���ڰѷ��ͻ������ڻ�������ݷ��͸��ͻ���
#define  CLIENT_SEND_BUFF_TIME 200

//�ͻ�����������
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

	//������������ ��ʱ�ò���
	void SendData02(DataHeader* header)
	{
		SendData(header);
		SendDataReal();
	}

	//���������ͻ����������ݷ��͸��ͻ���
	int SendDataReal()
	{
		int ret = SOCKET_ERROR;
		//������������
		if (_lastSendPos>0&&SOCKET_ERROR!=_sockfd)
		{
			//��������
			ret = send(_sockfd, _szSendBuf, _lastSendPos, 0);
			//����β��λ������Ϊ0
			_lastSendPos = 0;
			_sendBuffFullCount = 0;
			resetDtSend();
		}
		return ret;
	}
	//��������
	int SendData(DataHeader* header)
	{
		int ret = SOCKET_ERROR;
		//Ҫ���͵����ݳ���
		int nSendLen = header->dataLength;
		//Ҫ���͵�����
		const char* pSendData = (const char*)header;

		while (true)
		{
			if (nSendLen+_lastSendPos<=SEND_BUFF_SIZE)
			{
				/*��Ҫ���͵����� �����������ͻ�����β��*/
				memcpy(_szSendBuf + _lastSendPos, pSendData, nSendLen);
				/*��������β��λ��*/
				_lastSendPos += nSendLen;
				if (_lastSendPos==SEND_BUFF_SIZE)
				{
					_sendBuffFullCount++;
				}
				return nSendLen;
			}
			else
			{
				_sendBuffFullCount++;
			}
		}
		return ret;
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
			printf("checkHeart dead=%d,time=%d\n", _sockfd, _dtHeart);
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
			//���������ͻ����������ݷ��ͳ�ȥ
			SendDataReal();
			//���÷��ͼ�ʱ
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
	//�ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SIZE];
	//��Ϣ������������β��λ��
	int _lastPos;

	//�ڶ������� ���ͻ�����
	char _szSendBuf[SEND_BUFF_SIZE];
	//���ͻ�����������β��λ��
	int _lastSendPos;

	//����������ʱ
	time_t _dtHeart;

	//�ϴη�����Ϣ��ʱ��
	time_t _dtSend;

	/*���ͻ���������д�����*/
	int _sendBuffFullCount = 0;
};


#endif // !_CellClient_hpp_



