#include "DB_UpdateLastServerIdx.h"
#include <sql.h>
#include <sqlext.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <string.h>
#include "ClientSession.h"
#include "Core/EventLoop.h"
#include "../GlobalConfig.h"
#include "Database/DbConnectionPool.h"


template<>
DbQueryBinding* DbQueryJob<AuthServer::DB_UpdateLastServerIdx>::dbBinding = nullptr;

template<>
const char* DbQueryJob<AuthServer::DB_UpdateLastServerIdx>::SQL_CONFIG_NAME = "db_updatelastserveridx";

template<>
bool DbQueryJob<AuthServer::DB_UpdateLastServerIdx>::init(DbConnectionPool* dbConnectionPool) {
	std::vector<DbQueryBinding::ParameterBinding> params;
	std::vector<DbQueryBinding::ColumnBinding> cols;

	addParam(params, "last_login_server_idx", &InputType::lastLoginServerIdx);
	addParam(params, "account_id", &InputType::accountId);

	dbBinding = new DbQueryBinding(dbConnectionPool,
								   CFG_CREATE("sql.db_updatelastserveridx.enable", true),
								   CONFIG_GET()->auth.db.connectionString,
								   CFG_CREATE("sql.db_updatelastserveridx.query", "UPDATE account SET last_login_server_idx = ? WHERE account_id = ?;"),
								   params,
								   cols,
								   DbQueryBinding::EM_NoRow);

	return true;
}
