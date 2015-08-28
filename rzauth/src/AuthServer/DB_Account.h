#ifndef AUTHSERVER_DB_ACCOUNT_H
#define AUTHSERVER_DB_ACCOUNT_H

#include "Database/DbQueryJobCallback.h"
#include <string>
#include <string.h>
#include <stdint.h>

class DbConnectionPool;
class DesPasswordCipher;

namespace AuthServer {

class ClientSession;

struct DB_AccountData
{
	enum EncryptMode {
		EM_None,
		EM_DES,
		EM_AES
	};

	struct Input
	{
		std::string account;
		char ip[INET_ADDRSTRLEN];
		std::vector<unsigned char> cryptedPassword;
		EncryptMode cryptMode;
		unsigned char aesKey[32];

		// Computed in preProcess()
		char password[33];

		Input() {}
		Input(const std::string& account, const char* ip, EncryptMode cryptMode, const std::vector<unsigned char> &cryptedPassword, unsigned char aesKey[32])
			: account(account), cryptedPassword(cryptedPassword), cryptMode(cryptMode)
		{
			strncpy(this->ip, ip, INET_ADDRSTRLEN);
			this->ip[INET_ADDRSTRLEN-1] = 0;

			memcpy(this->aesKey, aesKey, sizeof(this->aesKey));
		}
	};

	struct Output
	{
		bool ok;
		uint32_t account_id;
		char password[34];
		bool nullPassword;
		bool auth_ok;
		uint32_t age;
		uint16_t last_login_server_idx;
		uint32_t event_code;
		uint32_t pcbang;
		uint32_t server_idx_offset;
		bool block;

		Output() {
			ok = false;
			account_id = 0xFFFFFFFF;
			this->password[0] = '\0';
			nullPassword = true;
			auth_ok = true;
			age = 19;
			last_login_server_idx = 1;
			event_code = 0;
			pcbang = 0;
			server_idx_offset = 0;
			block = false;
		}
	};
};

class DB_Account : public DbQueryJobCallback<DB_AccountData, ClientSession, DB_Account>
{
	DECLARE_CLASS(AuthServer::DB_Account)
public:
	static bool init(cval<std::string>& desKeyStr);
	static void deinit();

	DB_Account(ClientSession* clientInfo, DbQueryJobCallback::DbCallback callback);

protected:
	bool onPreProcess();
	bool onRowDone();
	bool isAccountNameValid(const std::string& account);
	bool decryptPassword();
	void setPasswordMD5(unsigned char givenPasswordMd5[16]);

private:
	static DesPasswordCipher* desCipher; //cached DES cipher
	static std::string currentDesKey;
	static cval<bool>* restrictCharacters;
};

} // namespace AuthServer

#endif // AUTHSERVER_DB_ACCOUNT_H
