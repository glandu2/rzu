#ifndef ENDPOINTPROXY_H
#define ENDPOINTPROXY_H

#include "IFilterEndpoint.h"
#include "Packet/PacketBaseMessage.h"

class EndpointProxy : public IFilterEndpoint {
public:
	EndpointProxy(IFilterEndpoint* endpoint);
	virtual ~EndpointProxy() {}

	virtual int getPacketVersion() override;
	virtual void sendPacket(const TS_MESSAGE* packet) override;
	virtual void close() override;
	virtual StreamAddress getAddress() override;
	virtual void banAddress(StreamAddress address) override;
	virtual bool isStrictForwardEnabled() override;

	void detach();

protected:
	virtual void sendPacket(MessageBuffer& packet) override;

private:
	IFilterEndpoint* endpoint;

	int cachedPacketVersion;
	StreamAddress cachedStreamAddress;
	bool cachedStrictForward;
};

#endif
