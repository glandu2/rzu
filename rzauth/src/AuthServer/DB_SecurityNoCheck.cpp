#include "DB_SecurityNoCheck.h"
#include "../GlobalConfig.h"
#include "ClientSession.h"
#include "Core/EventLoop.h"
#include "Database/DbConnectionPool.h"
#include "GameServerSession.h"
#include <openssl/md5.h>
#include <sql.h>
#include <sqlext.h>
#include <stdio.h>
#include <string.h>

template<> void DbQueryJob<AuthServer::DB_SecurityNoCheckData>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
	              CONFIG_GET()->auth.db.connectionString,
	              "SELECT account FROM account WHERE account = ? AND password = ?",
	              DbQueryBinding::EM_OneRow);

	addParam("account", &InputType::account);
	addParam("securityNoMd5String", &InputType::securityNoMd5String);
}
DECLARE_DB_BINDING(AuthServer::DB_SecurityNoCheckData, "db_securitynocheck");

namespace AuthServer {

cval<std::string>* DB_SecurityNoCheck::securityNoSalt = nullptr;

void DB_SecurityNoCheck::init() {
	securityNoSalt = &(CFG_CREATE("auth.securityno.salt", "2011"));
}

void DB_SecurityNoCheck::deinit() {
	securityNoSalt = nullptr;
}

DB_SecurityNoCheck::DB_SecurityNoCheck(GameServerSession* clientInfo, DbCallback callback)
    : DbQueryJobCallback(clientInfo, callback) {}

bool DB_SecurityNoCheck::onPreProcess() {
	if(!securityNoSalt) {
		log(LL_Warning, "Security No config not bound ! Can\'t check security no\n");
		return false;
	}

	unsigned char securityNoMd5[16];
	std::string buffer = securityNoSalt->get();
	DB_SecurityNoCheckData::Input* input = getInput();

	buffer += input->securityNo;
	MD5((const unsigned char*) buffer.c_str(), buffer.size(), securityNoMd5);
	setPasswordMD5(securityNoMd5);

	return true;
}

void DB_SecurityNoCheck::setPasswordMD5(unsigned char securityNoMd5[16]) {
	DB_SecurityNoCheckData::Input* input = getInput();

	for(int i = 0; i < 16; i++) {
		unsigned char val = securityNoMd5[i] >> 4;
		if(val < 10)
			input->securityNoMd5String[i * 2] = val + '0';
		else
			input->securityNoMd5String[i * 2] = val - 10 + 'a';

		val = securityNoMd5[i] & 0x0F;
		if(val < 10)
			input->securityNoMd5String[i * 2 + 1] = val + '0';
		else
			input->securityNoMd5String[i * 2 + 1] = val - 10 + 'a';
	}
	input->securityNoMd5String[32] = '\0';
}

}  // namespace AuthServer
