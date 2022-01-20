
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
	//内联函数
	/*还能读出N字节的数据吗*/
	inline bool canread(int n) 
	{
		return _nsize - _nreadpos >= n;
	}
	/*还能写出N字节的数据吗*/
	inline bool canwrite(int n)
	{
		return _nsize - _nwritepos >= n;
	}
	/*已写入位置,添加N字节的长度数据*/
	inline void push(int n)
	{
		_nwritepos += n;
	}
	/*已读取位置,添加N字节长度数据*/
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
		//计算要读取数据的字节长度
		size_t nlen = sizeof(t);
		/*判断能不能读*/
		if (canread(nlen))
		{
			/*将要读取的数据 拷贝出来*/
			memcpy(&n, _pbuff + _nreadpos, nlen);
			//计算已读数据的位置
			if (boffset)
			{
				pop(nlen);
			}
			return true;
		}
		/*断言*/
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
		/*读取数组元素个数*/
		uint32_t len1 = 0;
		/*读取数组元素个数,但不偏移读取位置*/
		Read(len1, false);
		/*判断缓存数组能否放得下*/
		if (len1 < len)
		{
			/*计算数组的实际字节长度*/
			auto nlen = len1 * sizeof(t);
			/*判断能不能读出*/
			if (canread(nlen+sizeof(uint32_t)))
			{
				/*计算已读的位置+数组长度所占空间*/
				pop(sizeof(uint32_t));
				/*将要读取的数据拷贝出来*/
				memcpy(parr, _pbuff + _nreadpos, nlen);
				/*计算已读数据位置*/
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
		//计算要写入数据的字节长度
		auto nlen = sizeof(t);
		//判断能不能写入
		if (canwrite(nlen))
		{
			//写入什么位置
			/*将要发送的数据 拷贝到缓冲区尾部*/
			memcpy(_pbuff + _nwritepos, &n, nlen);
			//计算已写入数据尾部位置
			push(nlen);
			return true;
		}
		CellLog::Info("CellStream ::Write1 failed");
		return false;
	}
	template<typename t>
	bool WriteArray(t* pdata, uint32_t len)
	{
		//计算要写入数组的字节长度
		size_t nlen = sizeof(t) * len;
		//判断能不能写入
		if (canwrite(nlen + sizeof(uint32_t)))
		{
			/*先写入数组元素的数量*/
			Write(len);
			//写入什么位置
			/*将要发送的数据 拷贝到缓冲区尾部*/
			memcpy(_pbuff + _nwritepos, pdata, nlen);
			//计算数据尾部位置
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
		return Write<float>(n);//可以不写<>这个
	}
	double WriteDouble(double n)
	{
		return Write<double>(n);//可以不写<>这个
	}
private:
	//数据缓冲区
	char* _pbuff = nullptr;
	//缓冲区总的空间大小,字节长度
	int _nsize = 0;;

	////已写入数据的尾部位置,已写入数据长度
	int _nwritepos = 0;
	//已读取数据的尾部位置
	int _nreadpos = 0;
	//pbuff是外部传入的数据块时是否应该被释放
	bool _bdelete = true;
};
#endif
