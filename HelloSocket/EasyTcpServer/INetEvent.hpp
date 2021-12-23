#ifndef _INetEvent_hpp
#define  _INetEvent_hpp
#include "Cell.hpp"
#include "CellClient.hpp"
class CellServer;//预定义
class INetEvent
{
public:
	/*客户端加入事件*/
	virtual void OnNetJoin(CellClient* pClient) = 0;
	/*客户端离开事件*/
	virtual void OnNetLeave(CellClient* pClient) = 0;//纯虚函数
	/*客户端消息事件*/
	virtual void OnNetMsg(CellServer* pCellServer, DataHeader* header, CellClient* pClient) = 0;//纯虚函数
	/*Recv事件*/
	virtual void OnNetRecv(CellClient* pClient) = 0;//虚函数
};
#endif // !_INetEvent_hpp
