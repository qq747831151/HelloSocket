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
	DataHeader()
	{
		dataLength = sizeof(DataHeader);
		cmd = CMD_ERROR;
	}
	short dataLength;//数据长度
	short cmd;//命令

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
	char data[32];
};
//登录结果
struct LoginResult :DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGINRESULT;
		result = 0;
	}
	int result;
	char data[92];
};
//登出
struct LoginOut :DataHeader
{
	LoginOut()
	{
		dataLength = sizeof(LoginOut);
		cmd = CMD_LOGINOUT;
	}
	char userName[32];

};
//登录结果
struct LoginOutResult :DataHeader
{
	LoginOutResult()
	{
		dataLength = sizeof(LoginOutResult);
		cmd = CMD_LOGINOUTRESULT;
		result = 0;
	}
	int result;
};

//新用户加入
struct LoginNewUser :DataHeader
{
	LoginNewUser()
	{
		dataLength = sizeof(LoginNewUser);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;
};