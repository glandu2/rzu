#ifndef DB_ACCOUNT_H
#define DB_ACCOUNT_H

#include "DbQueryJob.h"
#include <string>
#include <stdint.h>

class DbConnectionPool;

namespace AuthServer {

class ClientSession;

class DB_Account : public DbQueryJob<DB_Account>
{
	DECLARE_CLASS(AuthServer::DB_Account)

public:
	static bool init(DbConnectionPool* dbConnectionPool);
	static void deinit();

	DB_Account(ClientSession* clientInfo, const std::string& account, const char *password, size_t size);

	void cancel() { clientInfo = nullptr; DbQueryJob::cancel(); }

protected:
	bool onPreProcess();
	bool onRowDone();
	void onDone(Status status);

private:
	ClientSession* clientInfo;

	//Input
	std::string account;
	unsigned char givenPasswordMd5[16];
	char givenPasswordString[33];
	bool ok;

	//Output
	uint32_t accountId;
	uint32_t age;
	uint16_t lastServerIdx;
	uint32_t eventCode;
	uint32_t pcBang;
	uint32_t serverIdxOffset;
	bool block;
};

} // namespace AuthServer

#endif // DB_ACCOUNT_H
