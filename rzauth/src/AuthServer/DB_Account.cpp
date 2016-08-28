#include "DB_Account.h"
#include <openssl/md5.h>
#include "ClientSession.h"
#include "../GlobalConfig.h"
#include <openssl/evp.h>
#include <openssl/err.h>
#include "Cipher/DesPasswordCipher.h"

template<> void DbQueryJob<AuthServer::DB_AccountData>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
				  CONFIG_GET()->auth.db.connectionString,
				  "SELECT * FROM account WHERE account = ? AND password = ?;",
				  DbQueryBinding::EM_OneRow);

	addParam("account", &InputType::account);
	addParam("password", &InputType::password);
	addParam("ip", &InputType::ip);

	addColumn("account_id", &OutputType::account_id);
	addColumn("password", &OutputType::password, &OutputType::nullPassword);
	addColumn("auth_ok", &OutputType::auth_ok);
	addColumn("age", &OutputType::age);
	addColumn("last_login_server_idx", &OutputType::last_login_server_idx);
	addColumn("event_code", &OutputType::event_code);
	addColumn("pcbang", &OutputType::pcbang);
	addColumn("server_idx_offset", &OutputType::server_idx_offset);
	addColumn("block", &OutputType::block);
}
DECLARE_DB_BINDING(AuthServer::DB_AccountData, "db_account");

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
		EVP_CIPHER_CTX* d_ctx;
		int bytesWritten, totalLength;
		unsigned int bytesRead;

		log(LL_Debug, "Client login using AES\n");

		// if crypted size is > max size - 128 bits, then the decrypted password will overflow in the destination variable
		if(input->cryptedPassword.size() > sizeof(password) - 16) {
			log(LL_Warning, "RSA: invalid password length: %d\n", (int)input->cryptedPassword.size());
			return false;
		}

		d_ctx = EVP_CIPHER_CTX_new();
		if(!d_ctx)
			goto cleanup_aes;

		if(EVP_DecryptInit_ex(d_ctx, EVP_aes_128_cbc(), NULL, input->aesKey, input->aesKey + 16) <= 0)
			goto cleanup_aes;

		for(totalLength = bytesRead = 0; bytesRead + 15 < input->cryptedPassword.size(); bytesRead += 16) {
			if(EVP_DecryptUpdate(d_ctx, (unsigned char*)password + totalLength, &bytesWritten, &input->cryptedPassword[0] + bytesRead, 16) <= 0)
				goto cleanup_aes;
			totalLength += bytesWritten;
		}

		if(EVP_DecryptFinal_ex(d_ctx, (unsigned char*)password + totalLength, &bytesWritten) <= 0)
			goto cleanup_aes;

		totalLength += bytesWritten;

		if(totalLength >= (int)sizeof(password)) {
			log(LL_Error, "Password length overflow: %d >= %d\n", totalLength, (int)sizeof(password));
			goto cleanup_aes;
		}

		password[totalLength] = 0;
		ok = true;

	cleanup_aes:
		unsigned long errorCode = ERR_get_error();
		if(errorCode)
			log(LL_Warning, "AES: error while processing password for account %s: %s\n", input->account.c_str(), ERR_error_string(errorCode, nullptr));
		if(d_ctx)
			EVP_CIPHER_CTX_free(d_ctx);
	} else if(input->cryptMode == DB_AccountData::EM_None) {
		if(input->cryptedPassword.size() >= sizeof(password)) {
			log(LL_Error, "Password length overflow: %d >= %d\n", (int)input->cryptedPassword.size(), (int)sizeof(password));
		} else {
			memcpy(password, (char*)&input->cryptedPassword[0], input->cryptedPassword.size());
			password[input->cryptedPassword.size()] = 0;

			log(LL_Debug, "Client login using clear text\n");

			ok = true;
		}
	} else {
		if(input->cryptedPassword.size() >= sizeof(password)) {
			log(LL_Error, "Password length overflow: %d >= %d\n", (int)input->cryptedPassword.size(), (int)sizeof(password));
		} else {
			memcpy(password, (char*)&input->cryptedPassword[0], input->cryptedPassword.size());

			log(LL_Debug, "Client login using DES\n");
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
		log(LL_Debug, "Account name has invalid character: %s\n", input->account.c_str());
		return false;
	}

	if(decryptPassword() == false)
		return false;

	log(LL_Trace, "Querying for account \"%s\" and password MD5 \"%s\"\n", input->account.c_str(), input->password);

	return true;
}

bool DB_Account::onRowDone() {
	const DB_AccountData::Input* input = getInput();
	DB_AccountData::Output* output = getResults().back().get();

	if(output->account_id == 0xFFFFFFFF) {
		log(LL_Trace, "Account %s not found in database\n", input->account.c_str());
		return false;
	}

	if(output->nullPassword == true && output->password[0] == '\0') {
		output->ok = true;
	} else if(!strcmp(input->password, output->password)){
		output->ok = true;
	} else {
		log(LL_Trace, "Password mismatch for account \"%s\": client tried \"%s\", database has \"%s\"\n", input->account.c_str(), input->password, output->password);
	}

	return false;
}

} // namespace AuthServer
