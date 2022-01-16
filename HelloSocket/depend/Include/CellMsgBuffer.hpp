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
		////д��������ݲ�һ��Ҫ�ŵ��ڴ���
		////Ҳ���Դ洢�����ݿ���ߴ��̵ȴ洢����
		//if (_nLast + nLen > _nSize)
		//{
		//	//��Ҫд������ݴ��ڿ��ÿռ�
		//	int n = (_nLast + nLen) - _nSize;
		//	//��չBUFF
		//	if (n < 8192)
		//		n = 8192;
		//	char* buff = new char[_nSize+n];
		//	memcpy(buff, _pBuff, _nLast);
		//	delete[] _pBuff;
		//	_pBuff = buff;
		//}

		if (_nLast + nLen <= _nSize)
		{
			//��Ҫ���͵����� ���������ͻ�����β��
			memcpy(_pBuff + _nLast, pData, nLen);
			//��������β��λ��
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
		/*ʣ�������*/
		_nLast = n;
		if (_fullCount > 0)
			--_fullCount;
	}
	/*���������ͻ����������ݷ��͸��ͻ���*/
	int write2socket(SOCKET sockfd)
	{
		int ret = 0;
		//������������
		if (_nLast > 0 && INVALID_SOCKET != sockfd)
		{
			//��������
			ret = send(sockfd, _pBuff, _nLast, 0);
			//����β��λ������
			_nLast = 0;
			//
			_fullCount = 0;
		}
		return ret;
	}

	int read4socket(SOCKET sockfd)
	{
		//���пռ������
		if (_nSize - _nLast > 0)
		{
			//���տͻ�������
			char* szRecv = _pBuff + _nLast;
			//���ݴ浽szRecv��  �����������ɽ������ݵ���󳤶�
			int nLen = (int)recv(sockfd, szRecv, _nSize - _nLast, 0);//����ֵ�ǽ��յĳ���  revcz��mac����ֵ��long ����ǿתint
			//CELLLog::Info("nLen=%d\n", nLen);
			if (nLen <= 0)
			{
				return nLen;
			}
			//��Ϣ������������β��λ�ú���
			_nLast += nLen;
			return nLen;
		}
		return 0;
	}

	bool hasMsg()
	{
		//���ճ������
		//�ж���Ϣ�����������ݳ��ȴ�����Ϣͷnetmsg_DataHeader����
		if (_nLast >= sizeof(DataHeader))
		{
			//��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
			DataHeader* header = (DataHeader*)_pBuff;

			/*�������Ϊ�ֽ������ _pBuff�����cmd��dataLength�����ݶԻ��� ���ڻ���֪��������� ��ʱ��ô��*/
			if (header->cmd > header->dataLength)
			{
				DataHeader header1;
				header1.cmd = header->dataLength;
				header1.dataLength = header->cmd;
				header->cmd = header1.cmd;
				header->dataLength = header1.dataLength;
			}

			//�ж���Ϣ�����������ݳ��ȴ�����Ϣ����
			return _nLast >= header->dataLength;
		}
		return false;
	}

	bool needWrite()
	{
		return _nLast > 0;
	}
private:
	//�ڶ������� ���ͻ�����
	char* _pBuff = nullptr;
	//�������������������������ݿ�
	//list<char*> _pBuffList;
	//������������β��λ�ã��������ݳ���
	int _nLast = 0;
	//�������ܵĿռ��С���ֽڳ���
	int _nSize = 0;
	//������д����������
	int _fullCount = 0;
};

#endif // !_CELL_BUFFER_HPP_
