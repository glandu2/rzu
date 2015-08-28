#include "DB_SecurityNoCheck.h"
#include <sql.h>
#include <sqlext.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <string.h>
#include "ClientSession.h"
#include "Core/EventLoop.h"
#include "../GlobalConfig.h"
#include "Database/DbConnectionPool.h"
#include "GameServerSession.h"


template<>
DbQueryBinding* DbQueryJob<AuthServer::DB_SecurityNoCheckData>::dbBinding = nullptr;

template<>
bool DbQueryJob<AuthServer::DB_SecurityNoCheckData>::init(DbConnectionPool* dbConnectionPool) {
	std::vector<DbQueryBinding::ParameterBinding> params;
	std::vector<DbQueryBinding::ColumnBinding> cols;

	ADD_PARAM(params, "db_securitynocheck", account, 0, 1);
	ADD_PARAM(params, "db_securitynocheck", securityNoMd5String, 32, 2);

	dbBinding = new DbQueryBinding(dbConnectionPool,
								   CFG_CREATE("sql.db_securitynocheck.enable", true),
								   CONFIG_GET()->auth.db.connectionString,
								   CFG_CREATE("sql.db_securitynocheck.query", "SELECT account FROM account WHERE account = ? AND password = ?"),
								   params,
								   cols,
								   DbQueryBinding::EM_OneRow);

	return true;
}

namespace AuthServer {

cval<std::string>* DB_SecurityNoCheck::securityNoSalt = nullptr;

bool DB_SecurityNoCheck::init() {
	securityNoSalt = &(CFG_CREATE("auth.securityno.salt", "2011"));
	return true;
}

void DB_SecurityNoCheck::deinit() {
	securityNoSalt = nullptr;
}

DB_SecurityNoCheck::DB_SecurityNoCheck(GameServerSession* clientInfo, DbCallback callback)
	: DbQueryJobCallback(clientInfo, callback)
{
}

bool DB_SecurityNoCheck::onPreProcess() {
	if(!securityNoSalt) {
		warn("Security No config not bound ! Can\'t check security no\n");
		return false;
	}

	unsigned char securityNoMd5[16];
	std::string buffer = securityNoSalt->get();
	DB_SecurityNoCheckData::Input* input = getInput();

	buffer += input->securityNo;
	MD5((const unsigned char*)buffer.c_str(), buffer.size(), securityNoMd5);
	setPasswordMD5(securityNoMd5);

	return true;
}

void DB_SecurityNoCheck::setPasswordMD5(unsigned char securityNoMd5[16]) {
	DB_SecurityNoCheckData::Input* input = getInput();

	for(int i = 0; i < 16; i++) {
		unsigned char val = securityNoMd5[i] >> 4;
		if(val < 10)
			input->securityNoMd5String[i*2] = val + '0';
		else
			input->securityNoMd5String[i*2] = val - 10 + 'a';

		val = securityNoMd5[i] & 0x0F;
		if(val < 10)
			input->securityNoMd5String[i*2+1] = val + '0';
		else
			input->securityNoMd5String[i*2+1] = val - 10 + 'a';
	}
	input->securityNoMd5String[32] = '\0';
}

} // namespace AuthServer
