#ifndef _CELLClient_HPP_
#define _CELLClient_HPP_

#include"Cell.hpp"
#include"CellMsgBuffer.hpp"

//�ͻ����������������ʱʱ��
#define CLIENT_HREAT_DEAD_TIME 60000
//�ڼ��ָ��ʱ���
//�ѷ��ͻ������ڻ������Ϣ���ݷ��͸��ͻ���
#define CLIENT_SEND_BUFF_TIME 200
//�ͻ�����������
class CellClient
{
public:
	int id = -1;
	//����serverid
	int serverId = -1;
public:
	CellClient(SOCKET sockfd = INVALID_SOCKET) :
		_sendBuff(SEND_BUFF_SIZE),
		_recvBuff(RECV_BUFF_SIZE)
	{
		static int n = 1;
		id = n++;
		_sockfd = sockfd;

		resetDTHeart();
		resetDTSend();
	}

	~CellClient()
	{
		CellLog::Info("s=%d CELLClient%d.~CELLClient\n", serverId, id);
		if (INVALID_SOCKET != _sockfd)
		{
#ifdef _WIN32
			closesocket(_sockfd);
#else
			close(_sockfd);
#endif
			_sockfd = INVALID_SOCKET;
		}
	}


	SOCKET sockfd()
	{
		return _sockfd;
	}

	int RecvData()
	{
		return _recvBuff.read4socket(_sockfd);
	}

	bool hasMsg()
	{
		return _recvBuff.hasMsg();
	}

	DataHeader* front_msg()
	{
		return (DataHeader*)_recvBuff.data();
	}

	void pop_front_msg()
	{
		if (hasMsg())
			_recvBuff.pop(front_msg()->dataLength);
	}

	bool needWrite()
	{
		return _sendBuff.needWrite();
	}

	//���������ͻ����������ݷ��͸��ͻ���
	int SendDataReal()
	{
		resetDTSend();
		return _sendBuff.write2socket(_sockfd);
	}

	//�������Ŀ��Ƹ���ҵ������Ĳ��������
	//��������
	int SendData(DataHeader* header)
	{                     //Ҫ���͵�����      //Ҫ���͵����ݳ���
		if (_sendBuff.push((const char*)header, header->dataLength))
		{
			return header->dataLength;
		}
		return SOCKET_ERROR;
	}
	/*�������������Ϊ0*/
	void resetDTHeart()
	{
		_dtHeart = 0;
	}
	//���ϴη�����Ϣ��ʱ������Ϊ0
	void resetDTSend()
	{
		_dtSend = 0;
	}

	//�������
	bool checkHeart(time_t dt)
	{
		_dtHeart += dt;
		if (_dtHeart >= CLIENT_HREAT_DEAD_TIME)
		{
			CellLog::Info("checkHeart dead:s=%d,time=%ld\n", _sockfd, _dtHeart);
			return true;
		}
		return false;
	}

	//��ʱ������Ϣ���
	bool checkSend(time_t dt)
	{
		_dtSend += dt;
		if (_dtSend >= CLIENT_SEND_BUFF_TIME)
		{
			//CELLLog::Info("checkSend:s=%d,time=%d\n", _sockfd, _dtSend);
			//���������ͻ����������ݷ��ͳ�ȥ
			SendDataReal();
			//���÷��ͼ�ʱ
			resetDTSend();
			return true;
		}
		return false;
	}
private:
	// socket fd_set  file desc set
	SOCKET _sockfd;
	//�ڶ������� ������Ϣ������
	CellMsgBuffer _recvBuff;
	//���ͻ�����
	CellMsgBuffer _sendBuff;
	//����������ʱ
	time_t _dtHeart;
	//�ϴη�����Ϣ���ݵ�ʱ�� 
	time_t _dtSend;
	//���ͻ���������д���������
	int _sendBuffFullCount = 0;
};

#endif // !_CELLClient_HPP_