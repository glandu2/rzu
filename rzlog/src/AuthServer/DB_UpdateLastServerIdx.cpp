#include "DB_UpdateLastServerIdx.h"
#include <sql.h>
#include <sqlext.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <string.h>
#include "ClientSession.h"
#include "EventLoop.h"
#include "../GlobalConfig.h"
#include "DbConnectionPool.h"

#ifdef _MSC_VER
#define strncasecmp strnicmp
#endif

template<> DbQueryBinding* DbQueryJob<AuthServer::DB_UpdateLastServerIdx>::dbBinding = nullptr;

namespace AuthServer {

struct DbUpdateLastServerIdxConfig {
	cval<bool>& enable;
	cval<std::string>& query;
	cval<int> &paramServerIdx, &paramAccountId;

	DbUpdateLastServerIdxConfig() :
		enable(CFG_CREATE("sql.db_updatelastserveridx.enable", true)),
		query(CFG_CREATE("sql.db_updatelastserveridx.query", "UPDATE account SET last_login_server_idx = ? WHERE account_id = ?;")),
		paramServerIdx(CFG_CREATE("sql.db_updatelastserveridx.param.serveridx", 1)),
		paramAccountId(CFG_CREATE("sql.db_updatelastserveridx.param.accountid", 2)) {}
};
static DbUpdateLastServerIdxConfig* config = nullptr;

bool DB_UpdateLastServerIdx::init(DbConnectionPool* dbConnectionPool) {
	std::vector<DbQueryBinding::ParameterBinding> params;
	std::vector<DbQueryBinding::ColumnBinding> cols;

	config = new DbUpdateLastServerIdxConfig;

	params.emplace_back(DECLARE_PARAMETER(DB_UpdateLastServerIdx, accountId, 0, config->paramAccountId));
	params.emplace_back(DECLARE_PARAMETER(DB_UpdateLastServerIdx, lastLoginServerIdx, 0, config->paramServerIdx));

	dbBinding = new DbQueryBinding(dbConnectionPool, config->enable, CONFIG_GET()->auth.db.connectionString, config->query, params, cols);

	return true;
}

void DB_UpdateLastServerIdx::deinit() {
	DbQueryBinding* binding = dbBinding;
	dbBinding = nullptr;
	delete binding;
	delete config;
}

DB_UpdateLastServerIdx::DB_UpdateLastServerIdx(uint32_t accountId, uint16_t lastLoginServerIdx)
	: accountId(accountId), lastLoginServerIdx(lastLoginServerIdx)
{
	execute(DbQueryBinding::EM_NoRow);
}

} // namespace AuthServer
