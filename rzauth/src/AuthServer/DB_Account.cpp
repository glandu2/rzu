#include "DB_Account.h"
#include <openssl/md5.h>
#include "ClientSession.h"
#include "../GlobalConfig.h"
#include <openssl/evp.h>
#include <openssl/err.h>
#include "DesPasswordCipher.h"

#ifdef _MSC_VER
#define strncasecmp strnicmp
#endif


template<> DbQueryBinding* DbQueryJob<AuthServer::DB_Account>::dbBinding = nullptr;

namespace AuthServer {

struct DbAccountConfig {
	cval<bool>& enable;
	cval<std::string>& query;
	cval<int> &paramAccount, &paramPassword, &paramIp;
	cval<std::string> &colAccountId, &colPassword, &colAuthOk, &colAge, &colLastServerIdx, &colEventCode, &colPcBang, &colServerIdxOffset, &colBlock;
	cval<bool> &restrictCharacters;

	DbAccountConfig() :
		enable(CFG_CREATE("sql.db_account.enable", true)),
		query(CFG_CREATE("sql.db_account.query", "SELECT * FROM account WHERE account = ? AND password = ?;")),
		paramAccount (CFG_CREATE("sql.db_account.param.account" , 1)),
		paramPassword(CFG_CREATE("sql.db_account.param.password", 2)),
		paramIp(CFG_CREATE("sql.db_account.param.ip", -1)),
		colAccountId    (CFG_CREATE("sql.db_account.column.accountid"    , "account_id")),
		colPassword     (CFG_CREATE("sql.db_account.column.password"    , "password")),
		colAuthOk       (CFG_CREATE("sql.db_account.column.authok"    , "auth_ok")),
		colAge          (CFG_CREATE("sql.db_account.column.age"          , "age")),
		colLastServerIdx(CFG_CREATE("sql.db_account.column.lastserveridx", "last_login_server_idx")),
		colEventCode    (CFG_CREATE("sql.db_account.column.eventcode"    , "event_code")),
		colPcBang    (CFG_CREATE("sql.db_account.column.pcbang"    , "pcbang")),
		colServerIdxOffset    (CFG_CREATE("sql.db_account.column.serveridxoffset", "server_idx_offset")),
		colBlock(CFG_CREATE("sql.db_account.column.block", "block")),
		restrictCharacters(CFG_CREATE("auth.clients.restrictchars", true))
	{}
};
static DbAccountConfig* config = nullptr;
DesPasswordCipher* DB_Account::desCipher = nullptr;
std::string DB_Account::currentDesKey;

bool DB_Account::init(DbConnectionPool* dbConnectionPool, cval<std::string>& desKeyStr) {
	std::vector<DbQueryBinding::ParameterBinding> params;
	std::vector<DbQueryBinding::ColumnBinding> cols;

	currentDesKey = desKeyStr.get();
	desCipher = new DesPasswordCipher(desKeyStr.get().c_str());
	config = new DbAccountConfig;

	params.emplace_back(DECLARE_PARAMETER(DB_Account, account, 0, config->paramAccount));
	params.emplace_back(DECLARE_PARAMETER(DB_Account, givenPasswordString, 32, config->paramPassword));
	params.emplace_back(DECLARE_PARAMETER(DB_Account, ip, 0, config->paramIp));

	cols.emplace_back(DECLARE_COLUMN(DB_Account, accountId, 0, config->colAccountId));
	cols.emplace_back(DECLARE_COLUMN_WITH_INFO(DB_Account, password, sizeof(((DB_Account*)(0))->password), config->colPassword, nullPassword));
	cols.emplace_back(DECLARE_COLUMN(DB_Account, authOk, 0, config->colAuthOk));
	cols.emplace_back(DECLARE_COLUMN(DB_Account, age, 0, config->colAge));
	cols.emplace_back(DECLARE_COLUMN(DB_Account, lastServerIdx, 0, config->colLastServerIdx));
	cols.emplace_back(DECLARE_COLUMN(DB_Account, eventCode, 0, config->colEventCode));
	cols.emplace_back(DECLARE_COLUMN(DB_Account, pcBang, 0, config->colPcBang));
	cols.emplace_back(DECLARE_COLUMN(DB_Account, serverIdxOffset, 0, config->colServerIdxOffset));
	cols.emplace_back(DECLARE_COLUMN(DB_Account, block, 0, config->colBlock));

	dbBinding = new DbQueryBinding(dbConnectionPool, config->enable, CONFIG_GET()->auth.db.connectionString, config->query, params, cols);

	return true;
}

void DB_Account::deinit() {
	DbQueryBinding* binding = dbBinding;
	dbBinding = nullptr;
	delete binding;
	delete config;
	delete desCipher;
	currentDesKey = "";
}

DB_Account::DB_Account(ClientSession* clientInfo, const std::string& account, const char* ip, EncryptMode cryptMode, const std::vector<unsigned char> &cryptedPassword, unsigned char aesKey[32])
	: clientInfo(clientInfo), account(account)
{
	strncpy(this->ip, ip, INET_ADDRSTRLEN);
	this->ip[INET_ADDRSTRLEN-1] = 0;
	this->cryptedPassword = cryptedPassword;
	memcpy(this->aesKey, aesKey, sizeof(this->aesKey));
	this->cryptMode = cryptMode;

	ok = false;
	accountId = 0xFFFFFFFF;
	this->password[0] = '\0';
	nullPassword = true;
	authOk = true;
	age = 19;
	lastServerIdx = 1;
	eventCode = 0;
	pcBang = 0;
	serverIdxOffset = 0;
	block = false;

	execute(DbQueryBinding::EM_OneRow);
}

bool DB_Account::decryptPassword() {
	char password[64+16];
	bool ok = false;

	if(cryptMode == EM_AES) {
		EVP_CIPHER_CTX d_ctx;
		int bytesWritten, totalLength;
		unsigned int bytesRead;

		debug("Client login using AES\n");

		// if crypted size is > max size - 128 bits, then the decrypted password will overflow in the destination variable
		if(cryptedPassword.size() > sizeof(password) - 16) {
			warn("RSA: invalid password length: %d\n", (int)cryptedPassword.size());
			return false;
		}

		EVP_CIPHER_CTX_init(&d_ctx);

		if(EVP_DecryptInit_ex(&d_ctx, EVP_aes_128_cbc(), NULL, aesKey, aesKey + 16) <= 0)
			goto cleanup_aes;
		if(EVP_DecryptInit_ex(&d_ctx, NULL, NULL, NULL, NULL) <= 0)
			goto cleanup_aes;

		for(totalLength = bytesRead = 0; bytesRead + 15 < cryptedPassword.size(); bytesRead += 16) {
			if(EVP_DecryptUpdate(&d_ctx, (unsigned char*)password + totalLength, &bytesWritten, &cryptedPassword[0] + bytesRead, 16) <= 0)
				goto cleanup_aes;
			totalLength += bytesWritten;
		}

		if(EVP_DecryptFinal_ex(&d_ctx, (unsigned char*)password + totalLength, &bytesWritten) <= 0)
			goto cleanup_aes;

		totalLength += bytesWritten;

		if(totalLength >= (int)sizeof(password)) {
			error("Password length overflow: %d >= %d\n", totalLength, (int)sizeof(password));
			goto cleanup_aes;
		}

		password[totalLength] = 0;
		ok = true;

	cleanup_aes:
		unsigned long errorCode = ERR_get_error();
		if(errorCode)
			warn("AES: error while processing password for account %s: %s\n", account.c_str(), ERR_error_string(errorCode, nullptr));
		EVP_CIPHER_CTX_cleanup(&d_ctx);
	} else if(cryptMode == EM_None) {
		if(cryptedPassword.size() >= sizeof(password)) {
			error("Password length overflow: %d >= %d\n", (int)cryptedPassword.size(), (int)sizeof(password));
		} else {
			memcpy(password, (char*)&cryptedPassword[0], cryptedPassword.size());
			password[cryptedPassword.size()] = 0;

			debug("Client login using clear text\n");

			ok = true;
		}
	} else {
		if(cryptedPassword.size() >= sizeof(password)) {
			error("Password length overflow: %d >= %d\n", (int)cryptedPassword.size(), (int)sizeof(password));
		} else {
			memcpy(password, (char*)&cryptedPassword[0], cryptedPassword.size());

			debug("Client login using DES\n");
			desCipher->decrypt(password, (int)cryptedPassword.size());
			password[cryptedPassword.size()] = 0;

			ok = true;
		}
	}

	if(ok == true) {
		unsigned char givenPasswordMd5[16];
		std::string buffer = CONFIG_GET()->auth.db.salt;

		buffer.append(password, password + strlen(password));
		MD5((const unsigned char*)buffer.c_str(), buffer.size(), givenPasswordMd5);
		setPasswordMD5(givenPasswordMd5);
	}

	return ok;
}

void DB_Account::setPasswordMD5(unsigned char givenPasswordMd5[16]) {
	for(int i = 0; i < 16; i++) {
		unsigned char val = givenPasswordMd5[i] >> 4;
		if(val < 10)
			givenPasswordString[i*2] = val + '0';
		else
			givenPasswordString[i*2] = val - 10 + 'a';

		val = givenPasswordMd5[i] & 0x0F;
		if(val < 10)
			givenPasswordString[i*2+1] = val + '0';
		else
			givenPasswordString[i*2+1] = val - 10 + 'a';
	}
	givenPasswordString[32] = '\0';
}

bool DB_Account::isAccountNameValid(const std::string& account) {
	if(account.size() == 0)
		return false;

	for(size_t i = 0; i < account.size(); i++) {
		if((account[i] < 'a' || account[i] > 'z') &&
			(account[i] < 'A' || account[i] > 'Z') &&
			(account[i] < '0' || account[i] > '9'))
		{
			return false;
		}
	}

	return true;
}

bool DB_Account::onPreProcess() {
	//Accounts with invalid names are refused
	if(config->restrictCharacters.get() && !isAccountNameValid(account)) {
		debug("Account name has invalid character: %s\n", account.c_str());
		return false;
	}

	if(decryptPassword() == false)
		return false;

	trace("Querying for account \"%s\" and password MD5 \"%s\"\n", account.c_str(), givenPasswordString);

	return true;
}

bool DB_Account::onRowDone() {
	if(accountId == 0xFFFFFFFF) {
		trace("Account %s not found in database\n", account.c_str());
		return false;
	}

	if(nullPassword == true && password[0] == '\0') {
		ok = true;
	} else if(!strcmp(givenPasswordString, password)){
		ok = true;
	} else {
		trace("Password mismatch for account \"%s\": client tried \"%s\", database has \"%s\"\n", account.c_str(), givenPasswordString, password);
	}

	return false;
}

void DB_Account::onDone(Status status) {
	if(status != S_Canceled && clientInfo)
		clientInfo->clientAuthResult(ok && authOk, account, accountId, age, lastServerIdx, eventCode, pcBang, serverIdxOffset, block);
}

} // namespace AuthServer
