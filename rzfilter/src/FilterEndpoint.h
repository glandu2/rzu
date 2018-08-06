#ifndef FILTERENDPOINT_H
#define FILTERENDPOINT_H

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

private:
	FilterProxy* filter;
	bool toClient;
};

#endif
