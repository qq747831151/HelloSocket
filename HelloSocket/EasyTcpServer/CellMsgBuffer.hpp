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
	/*��ջ*/
	bool push(const char*pData,int nLen)
	{
		if (nLen+_nlast<=_nSize)
		{
			/*��Ҫ���͵����� ���������ͻ�����β��*/
			memcpy(_pBuff+_nlast, pData, nLen);
			//��������β��λ��
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

	/*��ջ*/
	void pop(int nLen)
	{
		int n = _nlast - nLen;
		if (n>0)
		{
			memcpy(_pBuff, _pBuff + nLen, n);
		}
		/*ʣ������*/
		_nlast -= n;
		if (_BuffFullCount>0)
		{
			--_BuffFullCount;
		}
	}
	/*���������ͻ����������ݷ��͸��ͻ���*/
	int Write2Socket(SOCKET sockfd)
	{
		int ret = 0;
		/*������������*/
		if (_nlast>0&&0!=sockfd)
		{
			//��������
			ret = send(sockfd, _pBuff, _nlast, 0);
			//����β��λ������0
			_nlast = 0;
			//
			_BuffFullCount = 0;
		}
		return ret;
	}
	int read4Socket(SOCKET sockfd)
	{
		//���пռ������
		if (_nSize-_nlast>0)
		{
			//���տͻ�������
			char* szRecv = _pBuff + _nlast;
			//���ݴ浽szRecv��  �����������ɽ������ݵ���󳤶�
			int nlen = (int)recv(sockfd, szRecv, _nSize - _nlast, 0);//����ֵ�ǽ��յĳ���  revcz��mac����ֵ��long ����ǿתint
			if (nlen<=0)
			{
				return nlen;
			}
		//��Ϣ������������β��β��λ�ú���
			_nlast += nlen;
			return nlen;
		}
		return 0;
	}
	bool hasMsg()
	{
		//���ճ������
		//�ж���Ϣ�����������ݳ����Ƿ������ϢͷDataHeader����
		if (_nlast>=sizeof(DataHeader))
		{
			//��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
			DataHeader* header = (DataHeader*)_pBuff;
			//�ж���Ϣ��������ݳ��ȴ�����Ϣ����
			if (_nlast>=header->dataLength)
			{
				return true;
			}
		}
		return false;
	}
private:
	//�ڶ������� ���ͻ�����
	char* _pBuff = nullptr;
	////��Ϣ���������β��λ��,�������ݳ���
	int _nlast = 0;;
	//�������ܵĿռ��С,�ֽڳ���
	int _nSize = 0;;
	//������д����������
	int _BuffFullCount = 0;;
};

#endif