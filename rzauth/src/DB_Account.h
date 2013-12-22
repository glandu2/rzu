#ifndef DB_ACCOUNT_H
#define DB_ACCOUNT_H

#include "Object.h"
#include "uv.h"
#include <string>
#include <stdint.h>

class ClientInfo;

class DB_Account : public Object
{
	DECLARE_CLASS(DB_Account)

public:
	DB_Account(ClientInfo* clientInfo, const std::string& account, const char *password);

	static void onProcess(uv_work_t *req);
	static void onDone(uv_work_t *req, int status);

private:
	static void initializeConfig();

	ClientInfo* clientInfo;
	uv_work_t req;
	std::string account;
	bool ok;
	uint32_t accountId;
	unsigned char givenPasswordMd5[16];
};

#endif // DB_ACCOUNT_H
