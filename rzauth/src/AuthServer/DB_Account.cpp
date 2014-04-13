#include "DB_Account.h"
#include <sql.h>
#include <sqlext.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <string.h>
#include "ClientSession.h"
#include "EventLoop.h"
#include "../GlobalConfig.h"
#include "DbConnectionPool.h"

#ifdef _MSC_VER
#define strncasecmp strnicmp
#endif

namespace AuthServer {

DbConnectionPool* DB_Account::dbConnectionPool = new DbConnectionPool;

bool DB_Account::init() {
	//Check connection
	std::string connectionString = CONFIG_GET()->auth.dbAccount.connectionString.get();
	const char* dbQuery = "SELECT * FROM information_schema.tables WHERE table_schema = 'dbo' AND table_name = 'Account';";
	DbConnection* connection;

	Log::get()->log(Log::LL_Info, "DB_Account::init", 16, "Checking connection to %s\n", connectionString.c_str());

	connection = dbConnectionPool->getConnection(connectionString.c_str());
	if(!connection) {
		Log::get()->log(Log::LL_Error, "DB_Account::init", 16, "Failed to connect to auth database\n", connectionString.c_str());
		if(CONFIG_GET()->auth.dbAccount.ignoreInitCheck == false)
			return false;
		else
			return true;
	}

	if(!connection->execute(dbQuery)) {
		Log::get()->log(Log::LL_Error, "DB_Account::init", 16, "Failed to execute query %s\n", dbQuery);
		connection->releaseWithError();
		if(CONFIG_GET()->auth.dbAccount.ignoreInitCheck == false)
			return false;
		else
			return true;
	}


	if(!connection->fetch()) {
		Log::get()->log(Log::LL_Error, "DB_Account::init", 16, "Failed to fetch data of query %s\n", dbQuery);
		connection->releaseWithError();
		if(CONFIG_GET()->auth.dbAccount.ignoreInitCheck == false)
			return false;
		else
			return true;
	}

	connection->release();
	Log::get()->log(Log::LL_Info, "DB_Account::init", 16, "Auth database Ok\n");

	return true;
}

DB_Account::DB_Account(ClientSession* clientInfo, const std::string& account, const char* password) : clientInfo(clientInfo), account(account) {
	std::string buffer = CONFIG_GET()->auth.dbAccount.salt;
	req.data = this;
	ok = false;
	accountId = 0;
	buffer.append(password);
	trace("MD5 of \"%s\" with len %ld\n", buffer.c_str(), (long)buffer.size());
	MD5((const unsigned char*)buffer.c_str(), buffer.size(), givenPasswordMd5);
	uv_queue_work(EventLoop::getLoop(), &req, &onProcess, &onDone);
}

void DB_Account::cancel() {
	clientInfo = 0;
	uv_cancel((uv_req_t*)&req);
}

void DB_Account::onProcess(uv_work_t *req) {
	DB_Account* thisInstance = (DB_Account*) req->data;
	char password[33] = {0};
	char givenPassword[33];
	DbConnection* connection;

	connection = dbConnectionPool->getConnection(CONFIG_GET()->auth.dbAccount.connectionString.get().c_str());
	if(!connection)
		return;

	thisInstance->trace("Executing query\n");
	connection->bindParameter(1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, thisInstance->account.size(), 0, const_cast<char*>(thisInstance->account.c_str()), thisInstance->account.size(), nullptr);
	if(!connection->execute("SELECT TOP(1) account_id, password FROM dbo.Account WITH(NOLOCK) WHERE account = ?;")) {
		connection->releaseWithError(Log::LL_Info);
		return;
	}
	if(!connection->fetch()) {
		connection->closeCursor();
		connection->release();
		return;
	}

	thisInstance->trace("Getting data\n");
	connection->getData(1, SQL_C_LONG, &thisInstance->accountId, sizeof(thisInstance->accountId), NULL);
	connection->getData(2, SQL_C_CHAR, password, sizeof(password), NULL);

	//if(connection->fetch())
	connection->closeCursor();

	connection->release();

	for(int i = 0; i < 16; i++) {
		unsigned char val = thisInstance->givenPasswordMd5[i] >> 4;
		if(val < 10)
			givenPassword[i*2] = val + '0';
		else
			givenPassword[i*2] = val - 10 + 'a';

		val = thisInstance->givenPasswordMd5[i] & 0x0F;
		if(val < 10)
			givenPassword[i*2+1] = val + '0';
		else
			givenPassword[i*2+1] = val - 10 + 'a';
	}
	givenPassword[32] = '\0';

	thisInstance->debug("Account password md5: %s; DB md5: %s;\n", givenPassword, password);

	if(!strncasecmp(givenPassword, password, 16*2)) {
		thisInstance->ok = true;
		thisInstance->trace("Ok\n");
	} else {
		thisInstance->trace("Not Ok\n");
	}
}

void DB_Account::onDone(uv_work_t *req, int status) {
	DB_Account* thisInstance = (DB_Account*) req->data;

	if(thisInstance->clientInfo)
		thisInstance->clientInfo->clientAuthResult(thisInstance->ok, thisInstance->account, thisInstance->accountId, 19, 1, 0);
	delete thisInstance;
}

} // namespace AuthServer
