#ifndef AUTHSERVER_DB_ACCOUNT_H
#define AUTHSERVER_DB_ACCOUNT_H

#include "DbQueryJob.h"
#include <string>
#include <stdint.h>

class DbConnectionPool;
class DesPasswordCipher;

namespace AuthServer {

class ClientSession;

class DB_Account : public DbQueryJob<DB_Account>
{
	DECLARE_CLASS(AuthServer::DB_Account)
public:
	enum EncryptMode {
		EM_None,
		EM_DES,
		EM_AES
	};

public:
	static bool init(DbConnectionPool* dbConnectionPool, cval<std::string>& desKeyStr);
	static void deinit();

	DB_Account(ClientSession* clientInfo, const std::string& account, const char* ip, EncryptMode cryptMode, const std::vector<unsigned char>& cryptedPassword, unsigned char aesKey[32]);

	void cancel() { clientInfo = nullptr; DbQueryJob::cancel(); }

protected:
	bool onPreProcess();
	bool onRowDone();
	void onDone(Status status);
	bool isAccountNameValid(const std::string& account);
	bool decryptPassword();
	void setPasswordMD5(unsigned char givenPasswordMd5[16]);

private:
	static DesPasswordCipher* desCipher; //cached DES cipher
	static std::string currentDesKey;

	ClientSession* clientInfo;

	//Input
	std::string account;
	char ip[INET_ADDRSTRLEN];
	std::vector<unsigned char> cryptedPassword;
	EncryptMode cryptMode;
	unsigned char aesKey[32];

	char givenPasswordString[33];

	//Output
	bool ok;
	uint32_t accountId;
	char password[34];
	bool nullPassword;
	bool authOk;
	uint32_t age;
	uint16_t lastServerIdx;
	uint32_t eventCode;
	uint32_t pcBang;
	uint32_t serverIdxOffset;
	bool block;
};

} // namespace AuthServer

#endif // AUTHSERVER_DB_ACCOUNT_H
