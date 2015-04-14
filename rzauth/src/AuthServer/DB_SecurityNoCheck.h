#ifndef AUTHSERVER_DB_SECURITY_NO_CHECK_H
#define AUTHSERVER_DB_SECURITY_NO_CHECK_H

#include "uv.h"
#include <stdint.h>
#include "DbQueryJob.h"

class DbConnectionPool;

namespace AuthServer {

class GameServerSession;

class DB_SecurityNoCheck : public DbQueryJob<DB_SecurityNoCheck>
{
	DECLARE_CLASS(AuthServer::DB_SecurityNoCheck)

public:
	static bool init(DbConnectionPool* dbConnectionPool);
	static void deinit();

	DB_SecurityNoCheck(GameServerSession* gameServerSession, std::string account, std::string securityNo, int32_t mode);
	void cancel() { gameServerSession = nullptr; DbQueryJob::cancel(); }

	bool onPreProcess();
	bool onRowDone();
	void onDone(Status status);

protected:
	void setPasswordMD5(unsigned char securityNoMd5[16]);

private:
	GameServerSession* gameServerSession;
	std::string account;
	std::string securityNo;
	int32_t mode;
	bool securityNoOk;
	char securityNoMd5String[33];
};

} // namespace AuthServer

#endif // AUTHSERVER_DB_SECURITY_NO_CHECK_H
