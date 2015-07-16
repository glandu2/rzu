#ifndef LOGSERVER_DB_INSERTLOG_H
#define LOGSERVER_DB_INSERTLOG_H

#include "uv.h"
#include <stdint.h>
#include "DbQueryJob.h"

class DbConnectionPool;

namespace LogServer {

class DB_InsertLog : public DbQueryJob<DB_InsertLog>
{
	DECLARE_CLASS(LogServer::DB_InsertLog)

public:
	static bool init(DbConnectionPool* dbConnectionPool);
	static void deinit();

	struct LogData {
		SQL_TIMESTAMP_STRUCT date;
		uint32_t thread_id;
		uint16_t id;

		int64_t n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11;
		std::string s1, s2, s3, s4;
	};

	DB_InsertLog(LogData logData);

private:
	LogData logData;
	SQLLEN dateInfo;
};

} // namespace LogServer

#endif // LOGSERVER_DB_INSERTLOG_H
