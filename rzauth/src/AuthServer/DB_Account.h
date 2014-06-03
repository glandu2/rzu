#ifndef DB_ACCOUNT_H
#define DB_ACCOUNT_H

#include "Object.h"
#include "uv.h"
#include <string>
#include <stdint.h>
#include "Log.h"
#include <list>

namespace AuthServer {

class ClientSession;
class DbConnectionPool;

class DB_Account : public Object
{
	DECLARE_CLASS(AuthServer::DB_Account)

public:
	static bool init();

	DB_Account(ClientSession* clientInfo, const std::string& account, const char *password, size_t size);

	void cancel();

	static void onProcess(uv_work_t *req);
	static void onDone(uv_work_t *req, int status);

private:
	~DB_Account() {}

	//one sql env for all connection
	static DbConnectionPool* dbConnectionPool;

	ClientSession* clientInfo;
	uv_work_t req;
	std::string account;
	bool ok;
	unsigned char givenPasswordMd5[16];

	uint32_t accountId;
	uint32_t age;
	uint16_t lastLoginServerIdx;
	uint32_t eventCode;
};

} // namespace AuthServer

#endif // DB_ACCOUNT_H
