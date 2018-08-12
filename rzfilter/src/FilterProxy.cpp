#include "FilterProxy.h"
#include "FilterManager.h"

FilterProxy::FilterProxy(FilterManager* filterManager, IFilter::ServerType serverType)
    : filterManager(filterManager),
      filterModule(nullptr),
      serverType(serverType),
      client(nullptr),
      server(nullptr),
      toServerEndpoint(this, false),
      toClientEndpoint(this, true) {}

FilterProxy::~FilterProxy() {
	filterManager->unregisterFilter(this);
	if(filterModule)
		filterManager->destroyInternalFilter(filterModule);
}

void FilterProxy::onServerPacket(const TS_MESSAGE* packet) {
	bool forwardPacket = true;
	if(filterModule)
		forwardPacket = filterModule->onServerPacket(packet);

	if(forwardPacket)
		client->sendPacket(packet);
}

void FilterProxy::onClientPacket(const TS_MESSAGE* packet) {
	bool forwardPacket = true;
	if(filterModule)
		forwardPacket = filterModule->onClientPacket(packet);

	if(forwardPacket)
		server->sendPacket(packet);
}

void FilterProxy::bindEndpoints(IFilterEndpoint* client, IFilterEndpoint* server) {
	this->client = client;
	this->server = server;
	filterManager->reloadFilter(this);
}

void FilterProxy::recreateFilterModule(IFilter::CreateFilterFunction createFilterFunction,
                                       IFilter::DestroyFilterFunction destroyFilterFunction) {
	IFilter* oldFilter = filterModule;
	filterModule = createFilterFunction(client, server, serverType, oldFilter);
	if(oldFilter && destroyFilterFunction)
		destroyFilterFunction(oldFilter);
}
