#ifndef PACKETFILTER_H
#define PACKETFILTER_H

#include "IFilter.h"

class PacketFilter : public IFilter
{
public:
	PacketFilter();

	void sendChatMessage(IFilterEndpoint* client, const char* msg);

	virtual bool onServerPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet);
	virtual bool onClientPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet);
};

#endif // PACKETFILTER_H
