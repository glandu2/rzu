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

	connection->closeCursor();
	connection->release();
	Log::get()->log(Log::LL_Info, "DB_Account::init", 16, "Auth database Ok\n");

	return true;
}

DB_Account::DB_Account(ClientSession* clientInfo, const std::string& account, const char* password, size_t size) : clientInfo(clientInfo), account(account) {
	std::string buffer = CONFIG_GET()->auth.dbAccount.salt;
	req.data = this;
	ok = false;

	accountId = 0;
	age = 19;
	lastLoginServerIdx = 1;
	eventCode = 0;

	buffer.append(password, password + size);
	trace("MD5 of \"%.*s\" with len %ld\n", (long)buffer.size(), buffer.c_str(), (long)buffer.size());
	MD5((const unsigned char*)buffer.c_str(), buffer.size(), givenPasswordMd5);
	uv_queue_work(EventLoop::getLoop(), &req, &onProcess, &onDone);
}

void DB_Account::cancel() {
	clientInfo = 0;
	uv_cancel((uv_req_t*)&req);
}

void DB_Account::onProcess(uv_work_t *req) {
	DB_Account* thisInstance = (DB_Account*) req->data;
	char givenPasswordString[33];
	DbConnection* connection;

	//Accounts with @ before the name are banned accounts
	if(thisInstance->account.size() == 0 || thisInstance->account[0] == '@') {
		thisInstance->debug("Account name has invalid character at start: %s\n", thisInstance->account.c_str());
		return;
	}

	connection = dbConnectionPool->getConnection(CONFIG_GET()->auth.dbAccount.connectionString.get().c_str());
	if(!connection) {
		thisInstance->debug("Could not retrieve a DB connection from pool\n");
		return;
	}

	for(int i = 0; i < 16; i++) {
		unsigned char val = thisInstance->givenPasswordMd5[i] >> 4;
		if(val < 10)
			givenPasswordString[i*2] = val + '0';
		else
			givenPasswordString[i*2] = val - 10 + 'a';

		val = thisInstance->givenPasswordMd5[i] & 0x0F;
		if(val < 10)
			givenPasswordString[i*2+1] = val + '0';
		else
			givenPasswordString[i*2+1] = val - 10 + 'a';
	}
	givenPasswordString[32] = '\0';

	thisInstance->trace("Querying for account \"%s\" and password MD5 \"%s\"\n", thisInstance->account.c_str(), givenPasswordString);

	const int paramAccountIndex = CONFIG_GET()->sql.dbAccount.paramAccount.get();
	const int paramPasswordIndex = CONFIG_GET()->sql.dbAccount.paramPassword.get();

	if(paramAccountIndex > 0)
		connection->bindParameter(paramAccountIndex, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, thisInstance->account.size(), 0, const_cast<char*>(thisInstance->account.c_str()), thisInstance->account.size(), nullptr);

	if(paramPasswordIndex > 0)
		connection->bindParameter(paramPasswordIndex, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 32, 0, givenPasswordString, 32, nullptr);

	if(!connection->execute(CONFIG_GET()->sql.dbAccount.query.get().c_str())) {
		connection->releaseWithError(Log::LL_Info);
		return;
	}
	if(!connection->fetch()) {
		thisInstance->trace("Not Ok\n");
		connection->closeCursor();
		connection->release();
		return;
	}

	thisInstance->trace("Getting data\n");
	bool columnCountOk;
	const int columnCount = connection->getColumnNum(&columnCountOk);

	if(!columnCountOk) {
		connection->closeCursor();
		connection->releaseWithError(Log::LL_Error);
		return;
	}

	if(columnCount >= 1)
		connection->getData(1, SQL_C_LONG, &thisInstance->accountId, sizeof(thisInstance->accountId), NULL);
	else {
		thisInstance->fatal("No columns in result set for query \"%s\" !\n", CONFIG_GET()->sql.dbAccount.query.get().c_str());
		connection->closeCursor();
		connection->releaseWithError(Log::LL_Error);
	}

	if(columnCount >= 2)
		connection->getData(2, SQL_C_LONG, &thisInstance->age, sizeof(thisInstance->age), NULL);

	if(columnCount >= 3)
		connection->getData(3, SQL_C_SHORT, &thisInstance->lastLoginServerIdx, sizeof(thisInstance->lastLoginServerIdx), NULL);

	if(columnCount >= 4)
		connection->getData(4, SQL_C_LONG, &thisInstance->eventCode, sizeof(thisInstance->eventCode), NULL);

	//if(connection->fetch())
	connection->closeCursor();

	connection->release();

	thisInstance->ok = true;
	thisInstance->trace("Ok\n");
}

void DB_Account::onDone(uv_work_t *req, int status) {
	DB_Account* thisInstance = (DB_Account*) req->data;

	if(status >= 0 && thisInstance->clientInfo)
		thisInstance->clientInfo->clientAuthResult(thisInstance->ok, thisInstance->account, thisInstance->accountId, thisInstance->age, thisInstance->lastLoginServerIdx, thisInstance->eventCode);
	delete thisInstance;
}

} // namespace AuthServer
