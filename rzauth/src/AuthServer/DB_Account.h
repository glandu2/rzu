#ifndef DB_ACCOUNT_H
#define DB_ACCOUNT_H

#include "Object.h"
#include "uv.h"
#include <string>
#include <stdint.h>
#include "Log.h"

namespace AuthServer {

class ClientInfo;

class DB_Account : public Object
{
	DECLARE_CLASS(AuthServer::DB_Account)

public:
	static bool init();

	DB_Account(ClientInfo* clientInfo, const std::string& account, const char *password);

	static void onProcess(uv_work_t *req);
	static void onDone(uv_work_t *req, int status);

protected:
	static bool openConnection(const std::string &connectionString, void **hdbc, void **hstmt);
	static void closeConnection(void **hdbc, void **hstmt);
	static void checkError(Log::Level errorLevel, void **hdbc, void **hstmt);

private:
	//one sql env for all connection
	static void* henv;

	ClientInfo* clientInfo;
	uv_work_t req;
	std::string account;
	bool ok;
	uint32_t accountId;
	unsigned char givenPasswordMd5[16];
};

} // namespace AuthServer

#endif // DB_ACCOUNT_H
