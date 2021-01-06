#pragma once

#include "AuthClient/TS_AC_ACCOUNT_NAME.h"
#include "AuthClient/TS_CA_ACCOUNT.h"
#include "Cipher/AesPasswordCipher.h"
#include "Cipher/RsaCipher.h"
#include "IFilter.h"

class SpecificPacketConverter {
public:
	bool convertAuthPacketAndSend(IFilterEndpoint* client,
	                              IFilterEndpoint* server,
	                              const TS_MESSAGE* packet,
	                              bool isServerMsg);
	bool convertGamePacketAndSend(IFilterEndpoint* target, const TS_MESSAGE* packet, int version, bool isServerMsg);

protected:
	static bool isNormalVersion(const std::string& version);

private:
	AesPasswordCipher clientAesCipher;
	AesPasswordCipher serverAesCipher;
	RsaCipher serverRsaCipher;
	struct {
		bool useImbc;
		std::string account;
		std::vector<uint8_t> password;
		std::vector<TS_ACCOUNT_ADDITIONAL_INFO> additionalInfos;
		bool isBoraAccount;
	} account;
	int64_t startTime;
};
