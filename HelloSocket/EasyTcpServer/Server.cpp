#define WIN32_LEAN_AND_MEAN//不影响 windows.h 和 WinSock2.h 前后顺序 

#define _WINSOCK_DEPRECATED_NO_WARNINGS //这个用于 inet_ntoa   可以在右击项目属性 C/C++ 预处理里面 预处理定义添加

#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
/*为了可以在其他平台也可以使用 右键项目属性 选择链接器 附加依赖项 将ws2_32.lib 添加进去就行 这样就不需要 下面这些 */
#pragma  comment(lib,"ws2_32.lib")
struct DataPackage
{
	int age;
	char name[32];
};
int main()
{
	/*启动socket网络环境 2.x环境*/
	WORD ver = MAKEWORD(2, 2);//版本号
	WSADATA dat;
	WSAStartup(ver, &dat);//动态库需要写上那个lib

	//1.建立Socket API 建立简易TC服务端
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//2.绑定 用于接受客户端连接的网络端口
	sockaddr_in _sin = {  };
	_sin.sin_family = AF_INET;//ipv4
	_sin.sin_port = htons(4567);//套字节序应该是网络字节序
	_sin.sin_addr.S_un.S_addr = ADDR_ANY;//获取IP得操作交给内核
	//_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");如果项目只是内网使用的话可以使用127
	int ret = bind(sock, (struct sockaddr*)&_sin, sizeof(_sin));
	if (ret == SOCKET_ERROR)
	{
		printf("ERROR,绑定用于接受客户端连接的网络端口失败...\n");
	}
	else
	{
		printf("TURE,绑定用于接受客户端连接的网络端口成功.....\n");
	}
	//3.监听网络端口
	ret = listen(sock, 5);
	if (ret == SOCKET_ERROR)
	{
		printf("ERROR,监听网络端口失败...\n");
	}
	else
	{
		printf("TURE,监听网络端口成功.....\n");
	}
	//4.等待接受客户端连接
	sockaddr_in addCli = {};
	int nlen = sizeof(addCli);
	SOCKET _clientSocket = INVALID_SOCKET;//无效网址
	_clientSocket = accept(sock, ( sockaddr*)&addCli, &nlen);
	if (_clientSocket == INVALID_SOCKET)
	{
		printf("ERROR,等待接受客户端连接失败...\n");
	}
	printf("新客户端加入：socket=%d IP=%s\n", (int)_clientSocket, inet_ntoa(addCli.sin_addr));
	char cmBuf[128] = {};
	while (1)
	{
		//5.接受客户端请求数据
		int nlen = recv(_clientSocket, cmBuf, sizeof(cmBuf), 0);//返回值是接收的长度
		if (nlen<=0)
		{
			printf("客户端退出,任务结束");
			break;
		}
		printf("收到命令:%s\n", cmBuf);
		//6.处理请求
		if (strcmp(cmBuf,"getInfo")==0)
		{
			//7.想客户端发送一条数据
			struct DataPackage dp = { 80,"小强" };
			send(_clientSocket, (const char *)&dp, sizeof(DataPackage), 0);

		}
	
	}
	//8 关闭套接字
	closesocket(sock);
	WSACleanup();
	getchar();
	return 0;

}
