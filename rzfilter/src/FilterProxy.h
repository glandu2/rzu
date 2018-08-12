#ifndef FILTERPROXY_H
#define FILTERPROXY_H

#include "Core/Object.h"
#include "FilterEndpoint.h"
#include "IFilter.h"
#include "IFilterEndpoint.h"

class FilterManager;

class FilterProxy {
public:
	FilterProxy(FilterManager* filterManager, IFilter::ServerType serverType);
	~FilterProxy();

	void onServerPacket(const TS_MESSAGE* packet);
	void onClientPacket(const TS_MESSAGE* packet);

	void bindEndpoints(IFilterEndpoint* client, IFilterEndpoint* server);
	IFilterEndpoint* getToClientEndpoint() { return &toClientEndpoint; }
	IFilterEndpoint* getToServerEndpoint() { return &toServerEndpoint; }
	IFilterEndpoint* getClientEndpoint() { return client; }
	IFilterEndpoint* getServerEndpoint() { return server; }

	int getPacketVersion() { return server->getPacketVersion(); }

protected:
	void recreateFilterModule(IFilter::CreateFilterFunction createFilterFunction,
	                          IFilter::DestroyFilterFunction destroyFilterFunction);

	friend class FilterManager;

private:
	FilterManager* filterManager;
	IFilter* filterModule;
	IFilter::ServerType serverType;
	IFilterEndpoint* client;
	IFilterEndpoint* server;
	FilterEndpoint toServerEndpoint;
	FilterEndpoint toClientEndpoint;
};

#endif  // FILTERPROXY_H
