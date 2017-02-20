#ifndef IFILTER_H
#define IFILTER_H

#include "IFilterEndpoint.h"
#include "Packet/PacketBaseMessage.h"

class IFilter
{
public:
	enum ServerType {
		ST_Auth,
		ST_Game
	};

	virtual ~IFilter() {}
	virtual bool onServerPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet, ServerType serverType) { return true; }
	virtual bool onClientPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet, ServerType serverType) { return true; }
};

#endif // IFILTER_H
