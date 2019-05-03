#ifndef IFILTER_H
#define IFILTER_H

#include "IFilterEndpoint.h"
#include "Packet/PacketBaseMessage.h"

class IFilter {
public:
	enum ServerType { ST_Auth, ST_Game };

	typedef void (*DestroyFilterFunction)(IFilter* filter);
	typedef IFilter* (*CreateFilterFunction)(IFilterEndpoint* client,
	                                         IFilterEndpoint* server,
	                                         ServerType serverType,
	                                         IFilter* oldFilter);

	virtual ~IFilter() {}
	virtual bool onServerPacket(const TS_MESSAGE* packet) { return true; }
	virtual bool onClientPacket(const TS_MESSAGE* packet) { return true; }
	virtual void onServerDisconnected() { client->close(); }
	virtual void onClientDisconnected() { server->close(); }

protected:
	IFilter(IFilterEndpoint* client, IFilterEndpoint* server, ServerType serverType)
	    : client(client), server(server), serverType(serverType) {}
	IFilterEndpoint* client;
	IFilterEndpoint* server;
	ServerType serverType;
};

#endif  // IFILTER_H
