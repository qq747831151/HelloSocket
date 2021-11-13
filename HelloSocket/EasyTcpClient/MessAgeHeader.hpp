enum CMD
{
	CMD_LOGIN,
	CMD_LOGINRESULT,
	CMD_LOGINOUT,
	CMD_LOGINOUTRESULT,
	CMD_ERROR,
	CMD_NEW_USER_JOIN,

};
//消息头
struct DataHeader
{
	short cmd;//命令
	short dataLength;//数据长度
};
//登录
struct Login :DataHeader
{
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];
};
//登录结果
struct LoginResult :DataHeader
{
	LoginResult()
	{
		cmd = CMD_LOGINRESULT;
		dataLength = sizeof(LoginResult);
	}
	int result = 1;
	char data[1024];
};
//登出
struct LoginOut :DataHeader
{
	LoginOut()
	{
		cmd = CMD_LOGINOUT;
		dataLength = sizeof(LoginOut);

	}
	char userName[32];

};
//登录结果
struct LoginOutResult :DataHeader
{
	LoginOutResult()
	{
		cmd = CMD_LOGINOUTRESULT;
		dataLength = sizeof(LoginOutResult);
	}
	int result = 1;
};

//新用户加入
struct LoginNewUser :DataHeader
{
	LoginNewUser()
	{
		cmd = CMD_NEW_USER_JOIN;
		dataLength = sizeof(LoginNewUser);
		sock = 0;
	}
	int sock;
};