
#ifndef _CELL_HPP_
#define _CELL_HPP_

//SOCKET
#ifdef _WIN32
#define FD_SETSIZE      256
#define WIN32_LEAN_AND_MEAN//��Ӱ�� windows.h �� WinSock2.h ǰ��˳�� 
#define _WINSOCK_DEPRECATED_NO_WARNINGS//������� inet_ntoa   �������һ���Ŀ���� C/C++ Ԥ�������� Ԥ���������
#include<windows.h>
#include<WinSock2.h>
/*Ϊ�˿���������ƽ̨Ҳ����ʹ�� �Ҽ���Ŀ���� ѡ�������� ���������� ��ws2_32.lib ��ӽ�ȥ���� �����Ͳ���Ҫ ������Щ */
#pragma comment(lib,"ws2_32.lib")
#else
#include<unistd.h> //uni std
#include<arpa/inet.h>
#include<string.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif
//
#include"MessageHeader.hpp"
#include"CELLTimestamp.hpp"
#include"CellTask.hpp"
//
#include<stdio.h>

//��������С��Ԫ��С 10240��10k
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 8192
#define SEND_BUFF_SIZE 10240
#endif // !RECV_BUFF_SZIE

#endif // !_CELL_HPP