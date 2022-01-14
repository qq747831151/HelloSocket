#ifndef _INetEvent_hpp
#define  _INetEvent_hpp
#include "Cell.hpp"
#include "CellClient.hpp"
class CellServer;//Ԥ����
class INetEvent
{
public:
	/*�ͻ��˼����¼�*/
	virtual void OnNetJoin(CellClient* pClient) = 0;
	/*�ͻ����뿪�¼�*/
	virtual void OnNetLeave(CellClient* pClient) = 0;//���麯��
	/*�ͻ�����Ϣ�¼�*/
	virtual void OnNetMsg(CellServer* pCellServer, DataHeader* header, CellClient* pClient) = 0;//���麯��
	/*Recv�¼�*/
	virtual void OnNetRecv(CellClient* pClient) = 0;//�麯��
};
#endif // !_INetEvent_hpp
