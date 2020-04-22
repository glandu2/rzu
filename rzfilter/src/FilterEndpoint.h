#pragma once

#include "IFilterEndpoint.h"
#include "Packet/PacketBaseMessage.h"

class FilterProxy;

class FilterEndpoint : public IFilterEndpoint {
public:
	FilterEndpoint(FilterProxy* filter, bool toClient) : filter(filter), toClient(toClient) {}
	virtual ~FilterEndpoint() {}

	virtual int getPacketVersion() override;
	virtual void sendPacket(const TS_MESSAGE* packet) override;
	virtual void sendPacket(MessageBuffer& packet) override;
	virtual void close() override;
	virtual StreamAddress getAddress() override;
	virtual void banAddress(StreamAddress address) override;
	virtual bool isStrictForwardEnabled() override;

private:
	FilterProxy* filter;
	bool toClient;
};

