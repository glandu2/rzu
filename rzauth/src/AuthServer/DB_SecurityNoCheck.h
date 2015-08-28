#ifndef AUTHSERVER_DB_SECURITY_NO_CHECK_H
#define AUTHSERVER_DB_SECURITY_NO_CHECK_H

#include "uv.h"
#include <stdint.h>
#include "Database/DbQueryJobCallback.h"

class DbConnectionPool;

namespace AuthServer {

class GameServerSession;

struct DB_SecurityNoCheckData {
	struct Input {
		std::string account;
		std::string securityNo;
		int32_t mode;
		char securityNoMd5String[33];

		Input() {}
		Input(std::string account, std::string securityNo, int32_t mode) : account(account), securityNo(securityNo), mode(mode) {}
	};

	struct Output {};
};

class DB_SecurityNoCheck : public DbQueryJobCallback<DB_SecurityNoCheckData, GameServerSession, DB_SecurityNoCheck>
{
	DECLARE_CLASS(AuthServer::DB_SecurityNoCheck)

public:
	static bool init();
	static void deinit();

	DB_SecurityNoCheck(GameServerSession* clientInfo, DbCallback callback);

	bool onPreProcess();

protected:
	void setPasswordMD5(unsigned char securityNoMd5[16]);

private:
	static cval<std::string>* securityNoSalt;
};

} // namespace AuthServer

#endif // AUTHSERVER_DB_SECURITY_NO_CHECK_H
