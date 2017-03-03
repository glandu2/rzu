#ifndef AUTHPACKETCONVERTERFILTER_H
#define AUTHPACKETCONVERTERFILTER_H

#include "IFilter.h"
#include "LibGlobal.h"
#include "AuthClient/TS_CA_ACCOUNT.h"
#include "Cipher/AesPasswordCipher.h"
#include "Cipher/RsaCipher.h"

struct TS_SC_CHAT;

class AuthPacketConverterFilter : public IFilter
{
public:
	AuthPacketConverterFilter(AuthPacketConverterFilter* data);
	~AuthPacketConverterFilter();

	virtual bool onServerPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet, ServerType serverType);
	virtual bool onClientPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet, ServerType serverType);

protected:
	bool handleAuthPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet, bool isServerMsg);

private:
	AesPasswordCipher clientAesCipher;
	AesPasswordCipher serverAesCipher;
	RsaCipher serverRsaCipher;
	struct {
		std::string account;
		std::vector<uint8_t> password;
		std::vector<TS_ACCOUNT_ADDITIONAL_INFO> additionalInfos;
	} account;
};

extern "C" {
SYMBOL_EXPORT IFilter* createFilter(IFilter* oldFilter);
SYMBOL_EXPORT void destroyFilter(IFilter* filter);
}

#endif // AUTHPACKETCONVERTERFILTER_H
