#include "DB_Account.h"
#include <sql.h>
#include <sqlext.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <string.h>
#include "ClientInfo.h"

DB_Account::DB_Account(ClientInfo* clientInfo, const std::string& account, const char* password) : clientInfo(clientInfo), account(account) {
	std::string buffer = "2012";
	req.data = this;
	ok = false;
	accountId = 0;
	buffer.append(password);
	printf("MD5 of \"%s\" with len %zd\n", buffer.c_str(), buffer.size());
	MD5((const unsigned char*)buffer.c_str(), buffer.size(), givenPasswordMd5);
	uv_queue_work(uv_default_loop(), &req, &onProcess, &onDone);
}

void DB_Account::onProcess(uv_work_t *req) {
	DB_Account* thisInstance = (DB_Account*) req->data;
	SQLRETURN result;
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt;
	char password[33] = {0};
	char givenPassword[33];

	result = SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv);
	if(!SQL_SUCCEEDED(result)) return;

	result = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER);
	if(!SQL_SUCCEEDED(result)) {
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
		return;
	}

	printf("Connecting to SQLEXPRESS\n");
	SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
	result = SQLConnect(hdbc, (UCHAR*)"SQLEXPRESS", SQL_NTS, (UCHAR*)"sa", SQL_NTS, (UCHAR*)"", SQL_NTS);
	if(!SQL_SUCCEEDED(result)) {
		SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
		return;
	}

	SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

	printf("Executing query\n");
	//SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &thisInstance->accountId, 0, nullptr);
	SQLExecDirect(hstmt, (SQLCHAR*)"SELECT account_id, password FROM Auth82.dbo.Account WHERE account_id = 1;", SQL_NTS);
	if(!SQL_SUCCEEDED(SQLFetch(hstmt))) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
		return;
	}

	printf("Getting data\n");
	SQLGetData(hstmt, 1, SQL_C_LONG, &thisInstance->accountId, sizeof(thisInstance->accountId), NULL);
	SQLGetData(hstmt, 2, SQL_C_CHAR, (SQLCHAR*)password, sizeof(password), NULL);
	SQLCloseCursor(hstmt);

	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);

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

	printf("Account password md5: %s;\nDB md5: %s;\n", givenPassword, password);

	if(!strncasecmp(givenPassword, password, 16*2)) {
		thisInstance->ok = true;
		printf("Ok\n");
	}
}

void DB_Account::onDone(uv_work_t *req, int status) {
	DB_Account* thisInstance = (DB_Account*) req->data;

	thisInstance->clientInfo->clientAuthResult(thisInstance->ok, thisInstance->account, thisInstance->accountId, 18, 1, 0);
	delete thisInstance;
}
