#include "DB_Account.h"
#include <openssl/md5.h>
#include "ClientSession.h"
#include "../GlobalConfig.h"
#include <openssl/evp.h>
#include <openssl/err.h>
#include "Cipher/DesPasswordCipher.h"

template<>
DbQueryBinding* DbQueryJob<AuthServer::DB_AccountData>::dbBinding = nullptr;

template<>
bool DbQueryJob<AuthServer::DB_AccountData>::init(DbConnectionPool* dbConnectionPool) {
	std::vector<DbQueryBinding::ParameterBinding> params;
	std::vector<DbQueryBinding::ColumnBinding> cols;

	ADD_PARAM(params, "db_account", account, 0, 1);
	ADD_PARAM(params, "db_account", password, 32, 2);
	ADD_PARAM(params, "db_account", ip, 0, 3);

	ADD_COLUMN(cols, "db_account", account_id, 0);
	ADD_COLUMN_WITH_INFO(cols, "db_account", password, sizeof(((AuthServer::DB_AccountData::Output*)(0))->password), nullPassword);
	ADD_COLUMN(cols, "db_account", auth_ok, 0);
	ADD_COLUMN(cols, "db_account", age, 0);
	ADD_COLUMN(cols, "db_account", last_login_server_idx, 0);
	ADD_COLUMN(cols, "db_account", event_code, 0);
	ADD_COLUMN(cols, "db_account", pcbang, 0);
	ADD_COLUMN(cols, "db_account", server_idx_offset, 0);
	ADD_COLUMN(cols, "db_account", block, 0);

	dbBinding = new DbQueryBinding(dbConnectionPool,
								   CFG_CREATE("sql.db_account.enable", true),
								   CONFIG_GET()->auth.db.connectionString,
								   CFG_CREATE("sql.db_account.query", "SELECT * FROM account WHERE account = ? AND password = ?;"),
								   params,
								   cols,
								   DbQueryBinding::EM_OneRow);

	return true;
}

namespace AuthServer {

DesPasswordCipher* DB_Account::desCipher = nullptr;
std::string DB_Account::currentDesKey;
cval<bool>* DB_Account::restrictCharacters = nullptr;

bool DB_Account::init(cval<std::string>& desKeyStr) {
	currentDesKey = desKeyStr.get();
	desCipher = new DesPasswordCipher(desKeyStr.get().c_str());
	restrictCharacters = &(CFG_CREATE("auth.clients.restrictchars", true));

	return true;
}

void DB_Account::deinit() {
	delete desCipher;
	currentDesKey = "";
	restrictCharacters = nullptr;
}

DB_Account::DB_Account(ClientSession* clientInfo, DbCallback callback)
	: DbQueryJobCallback(clientInfo, callback)
{
}

bool DB_Account::decryptPassword() {
	char password[64+16];
	bool ok = false;
	DB_AccountData::Input* input = getInput();

	if(input->cryptMode == DB_AccountData::EM_AES) {
		EVP_CIPHER_CTX d_ctx;
		int bytesWritten, totalLength;
		unsigned int bytesRead;

		debug("Client login using AES\n");

		// if crypted size is > max size - 128 bits, then the decrypted password will overflow in the destination variable
		if(input->cryptedPassword.size() > sizeof(password) - 16) {
			warn("RSA: invalid password length: %d\n", (int)input->cryptedPassword.size());
			return false;
		}

		EVP_CIPHER_CTX_init(&d_ctx);

		if(EVP_DecryptInit_ex(&d_ctx, EVP_aes_128_cbc(), NULL, input->aesKey, input->aesKey + 16) <= 0)
			goto cleanup_aes;
		if(EVP_DecryptInit_ex(&d_ctx, NULL, NULL, NULL, NULL) <= 0)
			goto cleanup_aes;

		for(totalLength = bytesRead = 0; bytesRead + 15 < input->cryptedPassword.size(); bytesRead += 16) {
			if(EVP_DecryptUpdate(&d_ctx, (unsigned char*)password + totalLength, &bytesWritten, &input->cryptedPassword[0] + bytesRead, 16) <= 0)
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
			warn("AES: error while processing password for account %s: %s\n", input->account.c_str(), ERR_error_string(errorCode, nullptr));
		EVP_CIPHER_CTX_cleanup(&d_ctx);
	} else if(input->cryptMode == DB_AccountData::EM_None) {
		if(input->cryptedPassword.size() >= sizeof(password)) {
			error("Password length overflow: %d >= %d\n", (int)input->cryptedPassword.size(), (int)sizeof(password));
		} else {
			memcpy(password, (char*)&input->cryptedPassword[0], input->cryptedPassword.size());
			password[input->cryptedPassword.size()] = 0;

			debug("Client login using clear text\n");

			ok = true;
		}
	} else {
		if(input->cryptedPassword.size() >= sizeof(password)) {
			error("Password length overflow: %d >= %d\n", (int)input->cryptedPassword.size(), (int)sizeof(password));
		} else {
			memcpy(password, (char*)&input->cryptedPassword[0], input->cryptedPassword.size());

			debug("Client login using DES\n");
			desCipher->decrypt(password, (int)input->cryptedPassword.size());
			password[input->cryptedPassword.size()] = 0;

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
	DB_AccountData::Input* input = getInput();

	for(int i = 0; i < 16; i++) {
		unsigned char val = givenPasswordMd5[i] >> 4;
		if(val < 10)
			input->password[i*2] = val + '0';
		else
			input->password[i*2] = val - 10 + 'a';

		val = givenPasswordMd5[i] & 0x0F;
		if(val < 10)
			input->password[i*2+1] = val + '0';
		else
			input->password[i*2+1] = val - 10 + 'a';
	}
	input->password[32] = '\0';
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
	DB_AccountData::Input* input = getInput();

	//Accounts with invalid names are refused
	if(restrictCharacters && restrictCharacters->get() && !isAccountNameValid(input->account)) {
		debug("Account name has invalid character: %s\n", input->account.c_str());
		return false;
	}

	if(decryptPassword() == false)
		return false;

	trace("Querying for account \"%s\" and password MD5 \"%s\"\n", input->account.c_str(), input->password);

	return true;
}

bool DB_Account::onRowDone() {
	DB_AccountData::Input* input = getInput();
	DB_AccountData::Output* output = &getResults().back();

	if(output->account_id == 0xFFFFFFFF) {
		trace("Account %s not found in database\n", input->account.c_str());
		return false;
	}

	if(output->nullPassword == true && output->password[0] == '\0') {
		output->ok = true;
	} else if(!strcmp(input->password, output->password)){
		output->ok = true;
	} else {
		trace("Password mismatch for account \"%s\": client tried \"%s\", database has \"%s\"\n", input->account.c_str(), input->password, output->password);
	}

	return false;
}

} // namespace AuthServer
