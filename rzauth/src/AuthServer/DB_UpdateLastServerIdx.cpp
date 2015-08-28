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
bool DbQueryJob<AuthServer::DB_UpdateLastServerIdx>::init(DbConnectionPool* dbConnectionPool) {
	std::vector<DbQueryBinding::ParameterBinding> params;
	std::vector<DbQueryBinding::ColumnBinding> cols;

	ADD_PARAM(params, "db_updatelastserveridx", accountId, 0, 1);
	ADD_PARAM(params, "db_updatelastserveridx", lastLoginServerIdx, 0, 2);

	dbBinding = new DbQueryBinding(dbConnectionPool,
								   CFG_CREATE("sql.db_updatelastserveridx.enable", true),
								   CONFIG_GET()->auth.db.connectionString,
								   CFG_CREATE("sql.db_updatelastserveridx.query", "UPDATE account SET last_login_server_idx = ? WHERE account_id = ?;"),
								   params,
								   cols,
								   DbQueryBinding::EM_NoRow);

	return true;
}
