#include "DB_UpdateLastServerIdx.h"
#include "../GlobalConfig.h"
#include "ClientSession.h"
#include "Core/EventLoop.h"
#include "Database/DbConnectionPool.h"
#include <openssl/md5.h>
#include <sql.h>
#include <sqlext.h>
#include <stdio.h>
#include <string.h>

template<> void DbQueryJob<AuthServer::DB_UpdateLastServerIdx>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
	              CONFIG_GET()->auth.db.connectionString,
	              "UPDATE account SET last_login_server_idx = ? WHERE account_id = ?;",
	              DbQueryBinding::EM_NoRow);

	addParam("last_login_server_idx", &InputType::lastLoginServerIdx);
	addParam("account_id", &InputType::accountId);
}
DECLARE_DB_BINDING(AuthServer::DB_UpdateLastServerIdx, "db_updatelastserveridx");
