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
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, RECV_BUFF_SIZE);
		_lastPos = 0;

		memset(_szSendBuf, 0, SEND_BUFF_SIZE);
		_lastSendPos = 0;
		resetDtHeart();
		resetDtSend();
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
				//���÷��ͼ�ʱ
				resetDtSend();
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
};

#endif // !_CellClient_hpp_



