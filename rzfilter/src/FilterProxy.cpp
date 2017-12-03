#include "FilterProxy.h"
#include "FilterManager.h"

FilterProxy::FilterProxy(FilterManager* filterManager,
                         IFilterEndpoint* client,
                         IFilterEndpoint* server,
                         ServerType serverType)
    : IFilter(client, server, serverType), filterManager(filterManager), filterModule(nullptr) {}

FilterProxy::~FilterProxy() {
	if(filterModule)
		filterManager->destroyInternalFilter(filterModule);
}

bool FilterProxy::onServerPacket(const TS_MESSAGE* packet) {
	if(filterModule)
		return filterModule->onServerPacket(packet);
	else
		return IFilter::onServerPacket(packet);
}

bool FilterProxy::onClientPacket(const TS_MESSAGE* packet) {
	if(filterModule)
		return filterModule->onClientPacket(packet);
	else
		return IFilter::onClientPacket(packet);
}

void FilterProxy::recreateFilterModule(CreateFilterFunction createFilterFunction,
                                       DestroyFilterFunction destroyFilterFunction) {
	IFilter* oldFilter = filterModule;
	filterModule = createFilterFunction(client, server, serverType, oldFilter);
	if(oldFilter && destroyFilterFunction)
		destroyFilterFunction(oldFilter);
}
