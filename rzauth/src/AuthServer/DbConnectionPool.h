#ifndef DBCONNECTIONPOOL_H
#define DBCONNECTIONPOOL_H

#include "Object.h"
#include "uv.h"
#include <list>
#include <string>
#include <sqlext.h>
#include "Log.h"

namespace AuthServer {

class DbConnectionPool;

class DbConnection : public Object
{
	DECLARE_CLASS(AuthServer::DbConnection)
public:
	DbConnection(DbConnectionPool* conPool, void *hdbc, void *hstmt) : conPool(conPool), hdbc(hdbc), hstmt(hstmt) { uv_mutex_init(&lock); }
	~DbConnection() { SQLFreeHandle(SQL_HANDLE_STMT, hstmt); SQLFreeHandle(SQL_HANDLE_DBC, hdbc); uv_mutex_destroy(&lock); }

	bool trylock() { return uv_mutex_trylock(&lock) == 0; }
	void release() { uv_mutex_unlock(&lock); }
	void releaseWithError(Log::Level errorLevel = Log::LL_Error);

	bool bindParameter(SQLUSMALLINT       ipar,
					  SQLSMALLINT        fParamType,
					  SQLSMALLINT        fCType,
					  SQLSMALLINT        fSqlType,
					  SQLULEN            cbColDef,
					  SQLSMALLINT        ibScale,
					  SQLPOINTER         rgbValue,
					  SQLLEN             cbValueMax,
					  SQLLEN 		      *pcbValue)
	{
		return SQL_SUCCEEDED(SQLBindParameter(hstmt, ipar, fParamType, fCType, fSqlType, cbColDef, ibScale, rgbValue, cbValueMax, pcbValue));
	}

	bool execute(const char* query) {
		return SQL_SUCCEEDED(SQLExecDirect(hstmt, (SQLCHAR*)query, SQL_NTS));
	}

	bool fetch() {
		return SQL_SUCCEEDED(SQLFetch(hstmt));
	}

	int getColumnNum(bool *ok) {
		SQLSMALLINT colCount = 0;
		bool isOk = SQL_SUCCEEDED(SQLNumResultCols(hstmt, &colCount));

		if(ok != nullptr)
			*ok = isOk;

		return colCount;
	}

	bool getData(SQLUSMALLINT ColumnNumber,
				SQLSMALLINT TargetType,
				SQLPOINTER TargetValue,
				SQLLEN BufferLength,
				SQLLEN *StrLen_or_Ind)
	{
		return SQL_SUCCEEDED(SQLGetData(hstmt, ColumnNumber, TargetType, TargetValue, BufferLength, StrLen_or_Ind));
	}

	void closeCursor() {
		SQLCloseCursor(hstmt);
	}

private:
	uv_mutex_t lock;
	DbConnectionPool* conPool;
	SQLHDBC hdbc;
	SQLHSTMT hstmt;
};

class DbConnectionPool : public Object
{
	DECLARE_CLASSNAME(AuthServer::DbConnectionPool, 0)
public:
	DbConnectionPool();
	~DbConnectionPool();

	DbConnection* getConnection(const char* connectionString);
	DbConnection* addConnection(const char* connectionString, bool createLocked);
	void closeConnection(DbConnection* dbConnection);
	void extractError(Log::Level errorLevel);

private:
	std::list<DbConnection*> openedConnections;
	uv_mutex_t listLock;
	void* henv;
};


} // namespace AuthServer

#endif // DBCONNECTIONPOOL_H
