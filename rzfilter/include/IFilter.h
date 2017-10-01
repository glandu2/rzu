#ifndef IFILTER_H
#define IFILTER_H

#include "IFilterEndpoint.h"
#include "Packet/PacketBaseMessage.h"

class IFilter {
public:
	enum ServerType { ST_Auth, ST_Game };

	virtual ~IFilter() {}
	virtual bool onServerPacket(const TS_MESSAGE* packet, ServerType serverType) { return true; }
	virtual bool onClientPacket(const TS_MESSAGE* packet, ServerType serverType) { return true; }

protected:
	IFilter(IFilterEndpoint* client, IFilterEndpoint* server) : client(client), server(server) {}
	IFilterEndpoint* client;
	IFilterEndpoint* server;
};

#endif  // IFILTER_H
