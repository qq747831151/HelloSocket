#include "EasyTcpServer.hpp"

int main()
{

	EasyTcpServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
	while (server.IsRun())
	{
		server.OnRun();
	}
	server.Close();
	getchar();
	return 0;

}
