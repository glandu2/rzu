#include "DB_Account.h"
#include <sql.h>
#include <sqlext.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <string.h>
#include "ClientSession.h"
#include "EventLoop.h"
#include "../GlobalConfig.h"
#include "Log.h"

#ifdef _MSC_VER
#define strncasecmp strnicmp
#endif

namespace AuthServer {

struct DbConnection {
	HDBC hdbc;
	HSTMT hstmt;
};

static void extractError(Log::Level errorLevel, SQLHANDLE handle, SQLSMALLINT type);

void* DB_Account::henv = nullptr;
uv_key_t DB_Account::connectionKey;

bool DB_Account::init() {
	if(uv_key_create(&connectionKey) != 0) {
		Log::get()->log(Log::LL_Error, "DB_Account::init", 16, "Can\'t create TLS connection key\n");
		return false;
	}

	SQLRETURN result;
	result = SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv);
	if(!SQL_SUCCEEDED(result)) {
		return false;
	}
	result = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER);
	if(!SQL_SUCCEEDED(result)) {
		Log::get()->log(Log::LL_Error, "DB_Account::init", 16, "Can\'t use ODBC 3\n");
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
		return false;
	}

	//Check connection: TODO
	HDBC hdbc;
	HSTMT hstmt;
	std::string connectionString = CONFIG_GET()->auth.dbAccount.connectionString.get();
	const char* dbQuery = "SELECT * FROM information_schema.tables WHERE table_schema = 'dbo' AND table_name = 'Account';";

	Log::get()->log(Log::LL_Info, "DB_Account::init", 16, "Checking connection to %s\n", connectionString.c_str());
	if(!openConnection(connectionString, &hdbc, &hstmt)) {
		Log::get()->log(Log::LL_Error, "DB_Account::init", 16, "Failed to connect to auth database\n", connectionString.c_str());
		checkError(Log::LL_Error, &hdbc, &hstmt);
		if(CONFIG_GET()->auth.dbAccount.ignoreInitCheck == false)
			return false;
		else
			return true;
	}
	result = SQLExecDirect(hstmt, (SQLCHAR*)dbQuery, SQL_NTS);
	if(!SQL_SUCCEEDED(result)) {
		Log::get()->log(Log::LL_Error, "DB_Account::init", 16, "Failed to execute query %s\n", dbQuery);
		checkError(Log::LL_Error, &hdbc, &hstmt);
		if(CONFIG_GET()->auth.dbAccount.ignoreInitCheck == false)
			return false;
		else
			return true;
	}
	result = SQLFetch(hstmt);
	if(!SQL_SUCCEEDED(result)) {
		Log::get()->log(Log::LL_Error, "DB_Account::init", 16, "Failed to fetch data of query %s\n", dbQuery);
		checkError(Log::LL_Error, &hdbc, &hstmt);
		if(CONFIG_GET()->auth.dbAccount.ignoreInitCheck == false)
			return false;
		else
			return true;
	}

	closeConnection(&hdbc, &hstmt);
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
	HDBC hdbc;
	HSTMT hstmt;

	if(!openConnection(CONFIG_GET()->auth.dbAccount.connectionString.get(), &hdbc, &hstmt)) {
		checkError(Log::LL_Error, &hdbc, &hstmt);
		return;
	}

	thisInstance->trace("Executing query\n");
	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, thisInstance->account.size(), 0, (void*)thisInstance->account.c_str(), thisInstance->account.size(), nullptr);
	if(!SQL_SUCCEEDED(SQLExecDirect(hstmt, (SQLCHAR*)"SELECT TOP(1) account_id, password FROM dbo.Account WITH(NOLOCK) WHERE account = ?;", SQL_NTS))) {
		checkError(Log::LL_Info, &hdbc, &hstmt);
		return;
	}
	if(!SQL_SUCCEEDED(SQLFetch(hstmt))) {
		checkError(Log::LL_Debug, &hdbc, &hstmt);
		return;
	}

	thisInstance->trace("Getting data\n");
	SQLGetData(hstmt, 1, SQL_C_LONG, &thisInstance->accountId, sizeof(thisInstance->accountId), NULL);
	SQLGetData(hstmt, 2, SQL_C_CHAR, (SQLCHAR*)password, sizeof(password), NULL);

	if(SQLFetch(hstmt) == SQL_SUCCESS)
		SQLCloseCursor(hstmt);


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

	thisInstance->debug("Account password md5: %s;\nDB md5: %s;\n", givenPassword, password);

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

bool DB_Account::openConnection(const std::string& connectionString, void **hdbc, void **hstmt) {
	SQLRETURN result;

	//Connection already opened, use it
	DbConnection* dbConnection = (DbConnection*) uv_key_get(&connectionKey);
	if(dbConnection != nullptr) {
		*hdbc = dbConnection->hdbc;
		*hstmt = dbConnection->hstmt;
		return true;
	}

	*hdbc = nullptr;
	*hstmt = nullptr;

	//Log::get()->log(Log::LL_Trace, "DB_Account", "Connecting to %s\n", connectionString.c_str());
	result = SQLAllocHandle(SQL_HANDLE_DBC, henv, hdbc);
	if(!SQL_SUCCEEDED(result)) {
		return false;
	}

	result = SQLDriverConnect(*hdbc, nullptr, (UCHAR*)connectionString.c_str(), SQL_NTS, nullptr, 0, nullptr, 0);
	if(!SQL_SUCCEEDED(result)) {
		return false;
	}

	result = SQLAllocHandle(SQL_HANDLE_STMT, *hdbc, hstmt);
	if(!SQL_SUCCEEDED(result)) {
		return false;
	}

	//30 sec timeout
	SQLSetStmtAttr(*hstmt, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER)10, 0);
	SQLSetConnectAttr(*hdbc, SQL_ATTR_CONNECTION_TIMEOUT , (SQLPOINTER)10, 0);

	dbConnection = new DbConnection;
	dbConnection->hdbc = *hdbc;
	dbConnection->hstmt = *hstmt;
	uv_key_set(&connectionKey, dbConnection);

	return true;
}

void DB_Account::closeConnection(void **hdbc, void **hstmt) {
	DbConnection* dbConnection = (DbConnection*)uv_key_get(&connectionKey);
	if(dbConnection) {
		delete dbConnection;
		uv_key_set(&connectionKey, nullptr);
	}

	if(hstmt && *hstmt) {
		SQLCloseCursor(*hstmt);
		SQLFreeHandle(SQL_HANDLE_STMT, *hstmt);
		*hstmt = nullptr;
	}
	if(hdbc && *hdbc) {
		SQLDisconnect(*hdbc);
		SQLFreeHandle(SQL_HANDLE_DBC, *hdbc);
		*hdbc = nullptr;
	}
}

void DB_Account::checkError(Log::Level errorLevel, void **hdbc, void **hstmt) {
	if(hstmt && *hstmt)
		extractError(errorLevel, *hstmt, SQL_HANDLE_STMT);
	if(hdbc && *hdbc)
		extractError(errorLevel, *hdbc, SQL_HANDLE_DBC);

	extractError(errorLevel, henv, SQL_HANDLE_ENV);
	closeConnection(hdbc, hstmt);
}

static void extractError(Log::Level errorLevel, SQLHANDLE handle, SQLSMALLINT type) {
	SQLSMALLINT i = 0, len;
	SQLINTEGER native;
	SQLCHAR state[7];
	SQLCHAR text[256];
	SQLRETURN ret;

	do {
		ret = SQLGetDiagRec(type, handle, ++i, state, &native, text, sizeof(text), &len);
		if (SQL_SUCCEEDED(ret))
			Log::get()->log(errorLevel, "ODBCERROR", 9, "%s:%d:%ld:%s\n", state, i, (long)native, text);
	} while(ret == SQL_SUCCESS);
}

} // namespace AuthServer
