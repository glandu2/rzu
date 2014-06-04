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

DbConnectionPool* DB_Account::dbConnectionPool = nullptr;

bool DB_Account::init() {
	dbConnectionPool = new DbConnectionPool;

	//Check connection
	std::string connectionString = CONFIG_GET()->auth.dbAccount.connectionString.get();
	//const char* dbQuery = "SELECT * FROM information_schema.tables WHERE table_schema = 'dbo' AND table_name = 'Account';";
	std::string dbQuery = CONFIG_GET()->sql.dbAccount.query;
	const int paramAccountIndex = CONFIG_GET()->sql.dbAccount.paramAccount.get();
	const int paramPasswordIndex = CONFIG_GET()->sql.dbAccount.paramPassword.get();

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

	if(paramAccountIndex > 0)
		connection->bindParameter(paramAccountIndex, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 14, 0, (char*)"testconnection", 14, nullptr);

	if(paramPasswordIndex > 0)
		connection->bindParameter(paramPasswordIndex, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 14, 0, (char*)"testconnection", 14, nullptr);


	if(!connection->execute(dbQuery.c_str())) {
		Log::get()->log(Log::LL_Error, "DB_Account::init", 16, "Failed to execute query %s\n", dbQuery.c_str());
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

void DB_Account::deinit() {
	DbConnectionPool *conPool = dbConnectionPool;
	dbConnectionPool = nullptr;
	delete conPool;
}

DB_Account::DB_Account(ClientSession* clientInfo, const std::string& account, const char* password, size_t size) : clientInfo(clientInfo), account(account) {
	std::string buffer = CONFIG_GET()->auth.dbAccount.salt;
	req.data = this;
	ok = false;

	accountId = 0xFFFFFFFF;
	age = 19;
	lastLoginServerIdx = 1;
	eventCode = 0;

	buffer.append(password, password + size);
	trace("MD5 of \"%.*s\" with len %ld\n", (long)buffer.size(), buffer.c_str(), (long)buffer.size());
	MD5((const unsigned char*)buffer.c_str(), buffer.size(), givenPasswordMd5);

	if(dbConnectionPool != nullptr)
		uv_queue_work(EventLoop::getLoop(), &req, &onProcess, &onDone);
	else
		onDone(&req, 0);
}

void DB_Account::cancel() {
	clientInfo = 0;
	uv_cancel((uv_req_t*)&req);
}

void DB_Account::onProcess(uv_work_t *req) {
	DB_Account* thisInstance = (DB_Account*) req->data;
	char givenPasswordString[33];
	DbConnection* connection;
	bool columnCountOk;

	//Accounts with @ before the name are banned accounts
	if(thisInstance->account.size() == 0 || thisInstance->account[0] == '@') {
		thisInstance->debug("Account name has invalid character at start: %s\n", thisInstance->account.c_str());
		return;
	}

	connection = dbConnectionPool->getConnection(CONFIG_GET()->auth.dbAccount.connectionString.get().c_str());
	if(!connection) {
		thisInstance->warn("Could not retrieve a DB connection from pool\n");
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
		connection->releaseWithError();
		return;
	}
	if(!connection->fetch()) {
		thisInstance->trace("Not Ok\n");
		connection->release();
		return;
	}

	thisInstance->trace("Getting data\n");
	const int columnCount = connection->getColumnNum(&columnCountOk);

	if(!columnCountOk) {
		connection->releaseWithError();
		return;
	}

	for(int i = 0; i < columnCount; i++) {
		char columnName[32];
		const int columnIndex = i + 1;

		connection->getColumnName(columnIndex, columnName, sizeof(columnName));

		if(!strcmp(columnName, "account_id")) {
			connection->getData(columnIndex, SQL_C_LONG, &thisInstance->accountId, sizeof(thisInstance->accountId), NULL);

			//Ok only when we have the account id (password check in SQL query)
			thisInstance->ok = true;
		} else if(!strcmp(columnName, "age")) {
			connection->getData(columnIndex, SQL_C_LONG, &thisInstance->age, sizeof(thisInstance->age), NULL);
		} else if(!strcmp(columnName, "last_login_server_idx")) {
			connection->getData(columnIndex, SQL_C_SHORT, &thisInstance->lastLoginServerIdx, sizeof(thisInstance->lastLoginServerIdx), NULL);
		} else if(!strcmp(columnName, "event_code")) {
			connection->getData(columnIndex, SQL_C_LONG, &thisInstance->eventCode, sizeof(thisInstance->eventCode), NULL);
		}
	}

	if(!thisInstance->ok) {
		thisInstance->debug("Missing account_id column in resultset\n");
		thisInstance->trace("Not Ok\n");
	} else {
		thisInstance->trace("Ok: id %d, age %d, last_login_server_idx %d, event_code %d\n",
							thisInstance->accountId,
							thisInstance->age,
							thisInstance->lastLoginServerIdx,
							thisInstance->eventCode);
	}

	connection->release();
}

void DB_Account::onDone(uv_work_t *req, int status) {
	DB_Account* thisInstance = (DB_Account*) req->data;

	if(status >= 0 && thisInstance->clientInfo)
		thisInstance->clientInfo->clientAuthResult(thisInstance->ok, thisInstance->account, thisInstance->accountId, thisInstance->age, thisInstance->lastLoginServerIdx, thisInstance->eventCode);
	delete thisInstance;
}

} // namespace AuthServer
