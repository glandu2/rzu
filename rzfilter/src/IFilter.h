#ifndef IFILTER_H
#define IFILTER_H

#include "IFilterEndpoint.h"
#include "Packets/PacketBaseMessage.h"

class IFilter
{
public:
	virtual ~IFilter() {}
	virtual bool onServerPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet) { return true; }
	virtual bool onClientPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet) { return true; }
};

#endif // IFILTER_H
