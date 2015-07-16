#include "DB_InsertLog.h"
#include "../GlobalConfig.h"
#include "DbConnectionPool.h"

#ifdef _MSC_VER
#define strncasecmp strnicmp
#endif

template<> DbQueryBinding* DbQueryJob<LogServer::DB_InsertLog>::dbBinding = nullptr;

namespace LogServer {

struct DbInsertLogConfig {
	cval<bool>& enable;
	cval<std::string>& query;
	cval<int> &paramDate, &paramThreadId, &paramId;
	cval<int> &paramN1, &paramN2, &paramN3, &paramN4, &paramN5, &paramN6, &paramN7, &paramN8, &paramN9, &paramN10, &paramN11;
	cval<int> &paramS1, &paramS2, &paramS3, &paramS4;

	DbInsertLogConfig() :
		enable(CFG_CREATE("sql.db_insertlog.enable", true)),
		query(CFG_CREATE("sql.db_insertlog.query", "INSERT INTO log values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)")),
		paramDate(CFG_CREATE("sql.db_insertlog.param.date", 1)),
		paramThreadId(CFG_CREATE("sql.db_insertlog.param.thread_id", 2)),
		paramId(CFG_CREATE("sql.db_insertlog.param.id", 3)),
		paramN1(CFG_CREATE("sql.db_insertlog.param.n1", 4)),
		paramN2(CFG_CREATE("sql.db_insertlog.param.n2", 5)),
		paramN3(CFG_CREATE("sql.db_insertlog.param.n3", 6)),
		paramN4(CFG_CREATE("sql.db_insertlog.param.n4", 7)),
		paramN5(CFG_CREATE("sql.db_insertlog.param.n5", 8)),
		paramN6(CFG_CREATE("sql.db_insertlog.param.n6", 9)),
		paramN7(CFG_CREATE("sql.db_insertlog.param.n7", 10)),
		paramN8(CFG_CREATE("sql.db_insertlog.param.n8", 11)),
		paramN9(CFG_CREATE("sql.db_insertlog.param.n9", 12)),
		paramN10(CFG_CREATE("sql.db_insertlog.param.n10", 13)),
		paramN11(CFG_CREATE("sql.db_insertlog.param.n11", 14)),
		paramS1(CFG_CREATE("sql.db_insertlog.param.s1", 15)),
		paramS2(CFG_CREATE("sql.db_insertlog.param.s2", 16)),
		paramS3(CFG_CREATE("sql.db_insertlog.param.s3", 17)),
		paramS4(CFG_CREATE("sql.db_insertlog.param.s4", 18)) {}
};
static DbInsertLogConfig* config = nullptr;

bool DB_InsertLog::init(DbConnectionPool* dbConnectionPool) {
	std::vector<DbQueryBinding::ParameterBinding> params;
	std::vector<DbQueryBinding::ColumnBinding> cols;

	config = new DbInsertLogConfig;

	params.emplace_back(DECLARE_PARAMETER_WITH_INFO(DB_InsertLog, logData.date, sizeof(((DB_InsertLog*)(0))->logData.date), config->paramDate, dateInfo));
	params.emplace_back(DECLARE_PARAMETER(DB_InsertLog, logData.thread_id, 0, config->paramThreadId));
	params.emplace_back(DECLARE_PARAMETER(DB_InsertLog, logData.id, 0, config->paramId));
	params.emplace_back(DECLARE_PARAMETER(DB_InsertLog, logData.n1, 0, config->paramN1));
	params.emplace_back(DECLARE_PARAMETER(DB_InsertLog, logData.n2, 0, config->paramN2));
	params.emplace_back(DECLARE_PARAMETER(DB_InsertLog, logData.n3, 0, config->paramN3));
	params.emplace_back(DECLARE_PARAMETER(DB_InsertLog, logData.n4, 0, config->paramN4));
	params.emplace_back(DECLARE_PARAMETER(DB_InsertLog, logData.n5, 0, config->paramN5));
	params.emplace_back(DECLARE_PARAMETER(DB_InsertLog, logData.n6, 0, config->paramN6));
	params.emplace_back(DECLARE_PARAMETER(DB_InsertLog, logData.n7, 0, config->paramN7));
	params.emplace_back(DECLARE_PARAMETER(DB_InsertLog, logData.n8, 0, config->paramN8));
	params.emplace_back(DECLARE_PARAMETER(DB_InsertLog, logData.n9, 0, config->paramN9));
	params.emplace_back(DECLARE_PARAMETER(DB_InsertLog, logData.n10, 0, config->paramN10));
	params.emplace_back(DECLARE_PARAMETER(DB_InsertLog, logData.n11, 0, config->paramN11));
	params.emplace_back(DECLARE_PARAMETER(DB_InsertLog, logData.s1, 0, config->paramS1));
	params.emplace_back(DECLARE_PARAMETER(DB_InsertLog, logData.s2, 0, config->paramS2));
	params.emplace_back(DECLARE_PARAMETER(DB_InsertLog, logData.s3, 0, config->paramS3));
	params.emplace_back(DECLARE_PARAMETER(DB_InsertLog, logData.s4, 0, config->paramS4));

	dbBinding = new DbQueryBinding(dbConnectionPool, config->enable, CONFIG_GET()->log.db.connectionString, config->query, params, cols, DbQueryBinding::EM_NoRow);

	return true;
}

void DB_InsertLog::deinit() {
	DbQueryBinding* binding = dbBinding;
	dbBinding = nullptr;
	delete binding;
	delete config;
}

DB_InsertLog::DB_InsertLog(LogData logData)
	: logData(logData), dateInfo(sizeof(logData.date))
{
	execute();
}

} // namespace LogServer
