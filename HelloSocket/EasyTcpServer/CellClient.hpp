#ifndef _CellClient_hpp_
#define _CellClient_hpp_

#include"Cell.hpp"


//�ͻ����������������ʱʱ�� 60000����=60����
#define  CLIENT_HEARY_DEAD_TIME 60000

//�ͻ�����������
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
		resetDtHeart();
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
			//���� ��������
			if (_lastSendPos + nSendLen >= SEND_BUFF_SIZE)
			{
				//����ɿ��������ݳ���
				int nCopyLen = SEND_BUFF_SIZE - _lastSendPos;
				//��������
				memcpy(_szSendBuf + _lastSendPos, pSendData, nCopyLen);
				//����ʣ������λ��
				pSendData += nCopyLen;
				//����ʣ�����ݳ���
				nSendLen -= nCopyLen;
				//��������
				ret = send(_sockfd, _szSendBuf, SEND_BUFF_SIZE, 0);
				//����β��λ������
				_lastSendPos = 0;
				//���ʹ���
				if (SOCKET_ERROR == ret)
				{
					return ret;
				}
			}
			else {
				//��Ҫ���͵����� ���������ͻ�����β��
				memcpy(_szSendBuf + _lastSendPos, pSendData, nSendLen);
				//��������β��λ��
				_lastSendPos += nSendLen;
				break;
			}
		}
		return ret;
	}
	/*�������������Ϊ0*/
	void resetDtHeart()
	{
		_dtHeart = 0;
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
};

#endif // !_CellClient_hpp_



