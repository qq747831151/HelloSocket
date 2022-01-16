
#ifndef _CELL_MsgStream_Hpp_
#define _CELL_MsgStream_Hpp_
//#include "MessageHeader.hpp"
#include "CellStream.hpp"
///消息数据字节流BYTE
class CellReadStream:public CellStream
{
public:
	CellReadStream(DataHeader* header)
		:CellStream((char*)header,header->dataLength)
	{
		push(header->dataLength);
		/*预先读取消息长度*/
		//ReadInt16();
		/*预先读取消息命令*/
		//getNetCmd();
	}
	uint16_t getNetCmd()
	{
		uint16_t cmd = CMD_ERROR;
		Read<uint16_t>(cmd);
		return cmd;

	}
	void setNetcmd(uint16_t cmd)
	{
		Write(cmd);
	}


};
/*消息数据字节流*/
class CellWriteStream :public CellStream
{
public:
	CellWriteStream(char* pData, int nSize, bool bDelete = false)
		:CellStream(pData, nSize, bDelete)
	{
		/*预先占领消息长度所需空间*/
		Write<uint16_t>(0);
	}
	CellWriteStream(int nSize = 1024)
		:CellStream(nSize)
	{
		Write<uint16_t>(0);
	}
	void setNetCmd(uint16_t cmd)
	{
		Write<uint16_t>(cmd);
	}
	void finsh()
	{
		int pos = length();
		SetWritePos(0);
		Write<uint16_t>(pos);
		SetWritePos(pos);
	}
	bool WriteString(const char* str, int len)
	{
		return WriteArray(str, len);
	}
	bool WriteString(const char* str)
	{
		return WriteArray(str, strlen(str));
	}
	bool WriteString(std::string& str)
	{
		return WriteArray(str.c_str(), str.length());
	}
};
#endif