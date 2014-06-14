#include "DbConnectionPool.h"
#include "Log.h"
#include <stdlib.h>

static bool checkSqlResult(SQLRETURN result, const char* function, SQLHENV henv, SQLHDBC hdbc, SQLHSTMT hstmt);
static void outputError(Log::Level errorLevel, SQLHANDLE handle, SQLSMALLINT type);

DbConnectionPool::DbConnectionPool() {
	SQLRETURN result;

	uv_mutex_init(&listLock);

	result = SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv);
	if(!SQL_SUCCEEDED(result)) {
		fatal("Can\t allocate ODBC ENV handle\n");
		abort();
	}
	result = checkSqlResult(SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER),
							"SQLSetEnvAttr",
							henv, nullptr, nullptr);
	if(!result)
		error("Can\'t use ODBC 3\n");
}

DbConnectionPool::~DbConnectionPool() {
	std::list<DbConnection*>::iterator it, itEnd;

	info("Closing DB connections pool\n");

	for(it = openedConnections.begin(), itEnd = openedConnections.end(); it != itEnd; ++it) {
		DbConnection* connection = *it;
		delete connection;
	}
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
	uv_mutex_destroy(&listLock);
}

DbConnection* DbConnectionPool::getConnection(const char* connectionString) {
	DbConnection* dbConnection = nullptr;

	uv_mutex_lock(&listLock);
	std::list<DbConnection*>::iterator it, itEnd;
	for(it = openedConnections.begin(), itEnd = openedConnections.end(); it != itEnd; ++it) {
		DbConnection* connection = *it;
		if(connection->trylock()) {
			dbConnection = connection;
			break;
		}
	}
	uv_mutex_unlock(&listLock);

	if(dbConnection)
		return dbConnection;
	else
		return addConnection(connectionString, true);
}

DbConnection* DbConnectionPool::addConnection(const char* connectionString, bool createLocked) {
	SQLRETURN result;
	SQLHDBC hdbc = nullptr;
	SQLHSTMT hstmt = nullptr;

	debug("Connecting to %s\n", connectionString);

	result = checkSqlResult(SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc),
							"SQLAllocHandle",
							henv, nullptr, nullptr);
	if(!result)
		return nullptr;

	result = checkSqlResult(SQLDriverConnect(hdbc, nullptr, (UCHAR*)connectionString, SQL_NTS, nullptr, 0, nullptr, 0),
							"SQLDriverConnect",
							henv, hdbc, nullptr);
	if(!result) {
		error("Failed to connect to %s\n", connectionString);
		SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
		return nullptr;
	}

	result = checkSqlResult(SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt),
							"SQLAllocHandle",
							henv, hdbc, nullptr);
	if(!result) {
		error("Failed to alloc a resultset for %s\n", connectionString);
		SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
		return nullptr;
	}

	//20 sec timeout
	SQLSetStmtAttr(hstmt, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER)10, 0);
	SQLSetConnectAttr(hdbc, SQL_ATTR_CONNECTION_TIMEOUT , (SQLPOINTER)10, 0);

	DbConnection* dbConnection = new DbConnection(this, hdbc, hstmt);
	if(createLocked)
		dbConnection->trylock();

	uv_mutex_lock(&listLock);
	openedConnections.push_back(dbConnection);
	uv_mutex_unlock(&listLock);

	return dbConnection;
}

void DbConnectionPool::closeConnection(DbConnection* dbConnection) {
	uv_mutex_lock(&listLock);
	openedConnections.remove(dbConnection);
	uv_mutex_unlock(&listLock);

	dbConnection->deleteLater();
}

void DbConnection::releaseWithError() {
	conPool->closeConnection(this);
	release();
}

bool DbConnection::checkResult(SQLRETURN result, const char* function) {
	return checkSqlResult(result, function, conPool->getHenv(), hdbc, hstmt);
}

static bool checkSqlResult(SQLRETURN result, const char* function, SQLHENV henv, SQLHDBC hdbc, SQLHSTMT hstmt) {
	if(result == SQL_SUCCESS_WITH_INFO) {
		Log::get()->log(Log::LL_Info, "ODBC", 4, "%s: additional info:\n", function);
		if(hstmt)
			outputError(Log::LL_Info, hstmt, SQL_HANDLE_STMT);
		if(hdbc)
			outputError(Log::LL_Info, hdbc, SQL_HANDLE_DBC);
		if(henv)
			outputError(Log::LL_Info, henv, SQL_HANDLE_ENV);
	} else if(result == SQL_ERROR) {
		Log::get()->log(Log::LL_Error, "ODBC", 4, "%s: error:\n", function);
		if(hstmt)
			outputError(Log::LL_Error, hstmt, SQL_HANDLE_STMT);
		if(hdbc)
			outputError(Log::LL_Error, hdbc, SQL_HANDLE_DBC);
		if(henv)
			outputError(Log::LL_Error, henv, SQL_HANDLE_ENV);
	}

	return SQL_SUCCEEDED(result);
}

static void outputError(Log::Level errorLevel, SQLHANDLE handle, SQLSMALLINT type) {
	SQLSMALLINT i = 0, len;
	SQLINTEGER native;
	SQLCHAR state[7];
	SQLCHAR text[256];
	SQLRETURN ret;

	do {
		ret = SQLGetDiagRec(type, handle, ++i, state, &native, text, sizeof(text), &len);
		if (SQL_SUCCEEDED(ret))
			Log::get()->log(errorLevel, "ODBC", 4, "%s:%d:%ld:%s\n", state, i, (long)native, text);
	} while(ret == SQL_SUCCESS);
}