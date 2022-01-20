
#ifndef _cell_stream_hpp_
#define _cell_stream_hpp_
#include <cstdint>
#include "CellLog.hpp"
class CellStream
{
public:
	CellStream(char *pData,int nSize,bool bDelete=false)
	{
		_nsize = nSize;
		_pbuff = pData;
		_bdelete = bDelete;
	}
	CellStream(int nSize=1024)
	{
		_nsize = nSize;
		_pbuff = new char[_nsize];
		_bdelete = true;
	}
	virtual ~CellStream()
	{
		if (_pbuff && _bdelete)
		{
			delete[]_pbuff;
			_pbuff = nullptr;
		}
	}
	char* Data()
	{
		return _pbuff;
	}
	int length()
	{
		return _nwritepos;
	}
	//��������
	/*���ܶ���N�ֽڵ�������*/
	inline bool canread(int n) 
	{
		return _nsize - _nreadpos >= n;
	}
	/*����д��N�ֽڵ�������*/
	inline bool canwrite(int n)
	{
		return _nsize - _nwritepos >= n;
	}
	/*��д��λ��,���N�ֽڵĳ�������*/
	inline void push(int n)
	{
		_nwritepos += n;
	}
	/*�Ѷ�ȡλ��,���N�ֽڳ�������*/
	inline void pop(int n)
	{
		_nreadpos += n;
	}
	inline void SetWritePos(int n)
	{
		_nwritepos = n;
	}
	inline void SetReadPos(int n)
	{
		_nreadpos = n;
	}
	inline int GetWritePos()
	{
		return _nwritepos;
	}
	template<typename t>
	bool Read(t&n,bool boffset=true)
	{
		//����Ҫ��ȡ���ݵ��ֽڳ���
		size_t nlen = sizeof(t);
		/*�ж��ܲ��ܶ�*/
		if (canread(nlen))
		{
			/*��Ҫ��ȡ������ ��������*/
			memcpy(&n, _pbuff + _nreadpos, nlen);
			//�����Ѷ����ݵ�λ��
			if (boffset)
			{
				pop(nlen);
			}
			return true;
		}
		/*����*/
		CellLog::Info("CellStream ::Read1 failed");
		return false;
	}
	template<typename t>
	bool onlyread(t& n)
	{
		return Read(n, false);
	}
	template<typename t>
	uint32_t ReadArray(t*parr,uint32_t len)
	{
		/*��ȡ����Ԫ�ظ���*/
		uint32_t len1 = 0;
		/*��ȡ����Ԫ�ظ���,����ƫ�ƶ�ȡλ��*/
		Read(len1, false);
		/*�жϻ��������ܷ�ŵ���*/
		if (len1 < len)
		{
			/*���������ʵ���ֽڳ���*/
			auto nlen = len1 * sizeof(t);
			/*�ж��ܲ��ܶ���*/
			if (canread(nlen+sizeof(uint32_t)))
			{
				/*�����Ѷ���λ��+���鳤����ռ�ռ�*/
				pop(sizeof(uint32_t));
				/*��Ҫ��ȡ�����ݿ�������*/
				memcpy(parr, _pbuff + _nreadpos, nlen);
				/*�����Ѷ�����λ��*/
				pop(nlen);
				return len1;

			}
		}
		CellLog::Info(" Error CellStream ::Read2 failed");
		return 0;
	}
	/// read
	int8_t ReadInt8(int8_t n = 0)//char
	{
		Read(n);
		return n;
	}

	int16_t ReadInt16(int16_t n = 0)//char
	{
		Read(n);
		return n;
	}
	int32_t ReadInt32(int32_t n = 0)//int
	{
		Read(n);
		return n;
	}
	int64_t ReadInt64(int64_t n = 0)//long
	{

		Read(n);
		return n;

	}

	uint8_t ReadUInt8(uint8_t n = 0)//char
	{
		Read(n);
		return n;
	}

	uint16_t ReadUInt16(uint16_t n = 0)//char
	{

		Read(n);
		return n;
	}
	uint32_t ReadUInt32(uint32_t n = 0)//int
	{

		Read(n);
		return n;

	}
	uint64_t ReadUInt64(uint64_t n = 0)//long
	{

		Read(n);
		return n;

	}
	float ReadFloat(float n = 0.0f)
	{
		Read(n);
		return n;
	}
	double ReadDouble(double n = 0.0)
	{
		Read(n);
		return n;
	}


	///write
	template<typename t>
	bool Write(t n)
	{
		//����Ҫд�����ݵ��ֽڳ���
		auto nlen = sizeof(t);
		//�ж��ܲ���д��
		if (canwrite(nlen))
		{
			//д��ʲôλ��
			/*��Ҫ���͵����� ������������β��*/
			memcpy(_pbuff + _nwritepos, &n, nlen);
			//������д������β��λ��
			push(nlen);
			return true;
		}
		CellLog::Info("CellStream ::Write1 failed");
		return false;
	}
	template<typename t>
	bool WriteArray(t* pdata, uint32_t len)
	{
		//����Ҫд��������ֽڳ���
		size_t nlen = sizeof(t) * len;
		//�ж��ܲ���д��
		if (canwrite(nlen + sizeof(uint32_t)))
		{
			/*��д������Ԫ�ص�����*/
			Write(len);
			//д��ʲôλ��
			/*��Ҫ���͵����� ������������β��*/
			memcpy(_pbuff + _nwritepos, pdata, nlen);
			//��������β��λ��
			push(nlen);
			return true;
		}
		CellLog::Info("CellStream ::Write failed");
		return false;
	}
	//char
	bool WriteInt8(int8_t n)
	{
		return Write(n);
	}
	//short
	bool WriteInt16(int16_t n)
	{
		return Write(n);
	}
	//int
	bool WriteInt32(int32_t n)
	{
		return Write(n);
	}

	float  WriteFloat(float n)
	{
		return Write<float>(n);//���Բ�д<>���
	}
	double WriteDouble(double n)
	{
		return Write<double>(n);//���Բ�д<>���
	}
private:
	//���ݻ�����
	char* _pbuff = nullptr;
	//�������ܵĿռ��С,�ֽڳ���
	int _nsize = 0;;

	////��д�����ݵ�β��λ��,��д�����ݳ���
	int _nwritepos = 0;
	//�Ѷ�ȡ���ݵ�β��λ��
	int _nreadpos = 0;
	//pbuff���ⲿ��������ݿ�ʱ�Ƿ�Ӧ�ñ��ͷ�
	bool _bdelete = true;
};
#endif
