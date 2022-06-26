#pragma once

#include "IFilterEndpoint.h"
#include "Packet/PacketBaseMessage.h"
#include "Packet/PacketStructsName.h"

class IFilter {
public:
	typedef void (*DestroyGlobalFilterFunction)(void* argument);
	typedef void* (*InitializeGlobalFilterFunction)();
	typedef void (*DestroyFilterFunction)(IFilter* filter);
	typedef IFilter* (*CreateFilterFunction)(IFilterEndpoint* client,
	                                         IFilterEndpoint* server,
	                                         SessionType sessionType,
	                                         IFilter* oldFilter);

	virtual ~IFilter() {}
	virtual bool onServerPacket(const TS_MESSAGE* packet) { return true; }
	virtual bool onClientPacket(const TS_MESSAGE* packet) { return true; }
	virtual void onServerDisconnected() { client->close(); }
	virtual void onClientDisconnected() { server->close(); }

protected:
	IFilter(IFilterEndpoint* client, IFilterEndpoint* server, SessionType sessionType)
	    : client(client), server(server), sessionType(sessionType) {}
	IFilterEndpoint* client;
	IFilterEndpoint* server;
	SessionType sessionType;
};
