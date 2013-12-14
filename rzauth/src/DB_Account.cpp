#include "DB_Account.h"
#include <sql.h>
#include <sqlext.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <string.h>
#include "ClientInfo.h"
#include "Network/EventLoop.h"

static void extract_error(
		SQLHANDLE handle,
		SQLSMALLINT type)
{
	SQLINTEGER i = 0;
	SQLINTEGER native;
	SQLCHAR state[ 7 ];
	SQLCHAR text[256];
	SQLSMALLINT len;
	SQLRETURN ret;
	fprintf(stderr, "ODBC Error:\n");

	do
	{
		ret = SQLGetDiagRec(type, handle, ++i, state, &native, text,
							sizeof(text), &len );
		if (SQL_SUCCEEDED(ret))
			printf("%s:%d:%d:%s\n", state, i, native, text);
	}
	while( ret == SQL_SUCCESS );
}

DB_Account::DB_Account(ClientInfo* clientInfo, const std::string& account, const char* password) : clientInfo(clientInfo), account(account) {
	std::string buffer = "2012";
	req.data = this;
	ok = false;
	accountId = 0;
	buffer.append(password);
	log("MD5 of \"%s\" with len %zd\n", buffer.c_str(), buffer.size());
	MD5((const unsigned char*)buffer.c_str(), buffer.size(), givenPasswordMd5);
	uv_queue_work(EventLoop::getLoop(), &req, &onProcess, &onDone);
}

void DB_Account::onProcess(uv_work_t *req) {
	DB_Account* thisInstance = (DB_Account*) req->data;
	SQLRETURN result;
	SQLHENV henv = 0;
	SQLHDBC hdbc = 0;
	SQLHSTMT hstmt = 0;
	char password[33] = {0};
	char givenPassword[33];

	result = SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv);
	if(!SQL_SUCCEEDED(result))
		goto cleanup;

	result = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER);
	if(!SQL_SUCCEEDED(result)) {
		goto cleanup;
	}

	thisInstance->log("Connecting to SQLEXPRESS\n");
	result = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
	if(!SQL_SUCCEEDED(result))
		goto cleanup;

	result = SQLDriverConnect(hdbc, nullptr, (UCHAR*)"driver=FreeTDS;Server=192.168.1.16;Database=Arcadia82;UID=sa;PWD=;Port=1433;", SQL_NTS, nullptr, 0, nullptr, 0);
	if(!SQL_SUCCEEDED(result)) {
		printf("Error %d\n", result);
		goto cleanup;
	}

	SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

	thisInstance->log("Executing query\n");
	SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, thisInstance->account.size(), 0, (void*)thisInstance->account.c_str(), thisInstance->account.size(), nullptr);
	SQLExecDirect(hstmt, (SQLCHAR*)"SELECT account_id, password FROM Auth82.dbo.Account WHERE account_id = 1;", SQL_NTS);
	if(!SQL_SUCCEEDED(SQLFetch(hstmt))) {
		goto cleanup;
	}

	thisInstance->log("Getting data\n");
	SQLGetData(hstmt, 1, SQL_C_LONG, &thisInstance->accountId, sizeof(thisInstance->accountId), NULL);
	SQLGetData(hstmt, 2, SQL_C_CHAR, (SQLCHAR*)password, sizeof(password), NULL);


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

	thisInstance->log("Account password md5: %s;\nDB md5: %s;\n", givenPassword, password);

	if(!strncasecmp(givenPassword, password, 16*2)) {
		thisInstance->ok = true;
		thisInstance->log("Ok\n");
	}

cleanup:
	if(hstmt) {
		if(result == SQL_ERROR)
			extract_error(hstmt, SQL_HANDLE_STMT);
		SQLCloseCursor(hstmt);
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}
	if(hdbc) {
		if(result == SQL_ERROR)
			extract_error(hdbc, SQL_HANDLE_DBC);
		SQLDisconnect(hdbc);
		SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	}
	if(henv) {
		if(result == SQL_ERROR)
			extract_error(henv, SQL_HANDLE_ENV);
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
	}
}

void DB_Account::onDone(uv_work_t *req, int status) {
	DB_Account* thisInstance = (DB_Account*) req->data;

	thisInstance->clientInfo->clientAuthResult(thisInstance->ok, thisInstance->account, thisInstance->accountId, 18, 1, 0);
	delete thisInstance;
}
