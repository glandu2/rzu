#ifndef SPECIFICPACKETCONVERTERFILTER_H
#define SPECIFICPACKETCONVERTERFILTER_H

#include "IFilter.h"
#include "AuthClient/TS_CA_ACCOUNT.h"
#include "Cipher/AesPasswordCipher.h"
#include "Cipher/RsaCipher.h"

class SpecificPacketConverter
{
public:
	bool convertAuthPacketAndSend(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet, bool isServerMsg);
	bool convertGamePacketAndSend(IFilterEndpoint* target, const TS_MESSAGE* packet, int version, bool isServerMsg);

private:
	AesPasswordCipher clientAesCipher;
	AesPasswordCipher serverAesCipher;
	RsaCipher serverRsaCipher;
	struct {
		bool useImbc;
		std::string account;
		std::vector<uint8_t> password;
		std::vector<TS_ACCOUNT_ADDITIONAL_INFO> additionalInfos;
	} account;
	int64_t startTime;
};

#endif // SPECIFICPACKETCONVERTERFILTER_H
