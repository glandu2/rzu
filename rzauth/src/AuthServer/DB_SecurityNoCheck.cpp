#include "DB_SecurityNoCheck.h"
#include <sql.h>
#include <sqlext.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <string.h>
#include "ClientSession.h"
#include "EventLoop.h"
#include "../GlobalConfig.h"
#include "DbConnectionPool.h"
#include "GameServerSession.h"

template<> DbQueryBinding* DbQueryJob<AuthServer::DB_SecurityNoCheck>::dbBinding = nullptr;

namespace AuthServer {

struct DbSecurityNoCheckConfig : private IListener {
	cval<bool>& enable;
	cval<std::string>& query;
	cval<int> &paramAccount, &paramSecurityNo;
	cval<std::string> &securityNoSalt;

	DbSecurityNoCheckConfig() :
		enable(CFG_CREATE("sql.db_securitynocheck.enable", true)),
		query(CFG_CREATE("sql.db_securitynocheck.query", "SELECT account FROM account WHERE account = ? AND password = ?")),
		paramAccount(CFG_CREATE("sql.db_securitynocheck.param.account", 1)),
		paramSecurityNo(CFG_CREATE("sql.db_securitynocheck.param.securityno", 2)),
		securityNoSalt(CFG_CREATE("auth.securityno.salt", "2011"))
	{
		CONFIG_GET()->auth.db.salt.addListener(this, &updateSalt);
		updateSalt(this, &CONFIG_GET()->auth.db.salt);
	}

	static void updateSalt(IListener* instance, cval<std::string>* other) {
		DbSecurityNoCheckConfig* thisInstance = (DbSecurityNoCheckConfig*) instance;
		thisInstance->securityNoSalt.setDefault(other->get());
	}
};
static DbSecurityNoCheckConfig* config = nullptr;

bool DB_SecurityNoCheck::init(DbConnectionPool* dbConnectionPool) {
	std::vector<DbQueryBinding::ParameterBinding> params;
	std::vector<DbQueryBinding::ColumnBinding> cols;

	config = new DbSecurityNoCheckConfig;

	params.emplace_back(DECLARE_PARAMETER(DB_SecurityNoCheck, account, 0, config->paramAccount));
	params.emplace_back(DECLARE_PARAMETER(DB_SecurityNoCheck, securityNoMd5String, 32, config->paramSecurityNo));

	dbBinding = new DbQueryBinding(dbConnectionPool, config->enable, CONFIG_GET()->auth.db.connectionString, config->query, params, cols);

	return true;
}

void DB_SecurityNoCheck::deinit() {
	DbQueryBinding* binding = dbBinding;
	dbBinding = nullptr;
	delete binding;
	delete config;
}

DB_SecurityNoCheck::DB_SecurityNoCheck(GameServerSession* gameServerSession, std::string account, std::string securityNo, int32_t mode)
	: gameServerSession(gameServerSession), account(account), securityNo(securityNo), mode(mode), securityNoOk(false)
{
	execute(DbQueryBinding::EM_OneRow);
}

bool DB_SecurityNoCheck::onRowDone() {
	securityNoOk = true;
	return true;
}

bool DB_SecurityNoCheck::onPreProcess() {
	unsigned char securityNoMd5[16];
	std::string buffer = config->securityNoSalt;

	buffer += securityNo;
	MD5((const unsigned char*)buffer.c_str(), buffer.size(), securityNoMd5);
	setPasswordMD5(securityNoMd5);

	return true;
}

void DB_SecurityNoCheck::setPasswordMD5(unsigned char securityNoMd5[16]) {
	for(int i = 0; i < 16; i++) {
		unsigned char val = securityNoMd5[i] >> 4;
		if(val < 10)
			securityNoMd5String[i*2] = val + '0';
		else
			securityNoMd5String[i*2] = val - 10 + 'a';

		val = securityNoMd5[i] & 0x0F;
		if(val < 10)
			securityNoMd5String[i*2+1] = val + '0';
		else
			securityNoMd5String[i*2+1] = val - 10 + 'a';
	}
	securityNoMd5String[32] = '\0';
}

void DB_SecurityNoCheck::onDone(IDbQueryJob::Status status) {
	if(gameServerSession)
		gameServerSession->onSecurityNoCheckResult(this, account, mode, securityNoOk);
}

} // namespace AuthServer
