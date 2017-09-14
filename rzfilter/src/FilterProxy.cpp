#include "FilterProxy.h"
#include "FilterManager.h"

FilterProxy::FilterProxy(FilterManager* filterManager, IFilterEndpoint* client, IFilterEndpoint* server)
    : IFilter(client, server),
      filterManager(filterManager),
      filterModule(nullptr)
{
}

FilterProxy::~FilterProxy()
{
	if(filterModule)
		filterManager->destroyInternalFilter(filterModule);
}

bool FilterProxy::onServerPacket(const TS_MESSAGE *packet, ServerType serverType)
{
	if(filterModule)
		return filterModule->onServerPacket(packet, serverType);
	else
		return IFilter::onServerPacket(packet, serverType);
}

bool FilterProxy::onClientPacket(const TS_MESSAGE *packet, ServerType serverType)
{
	if(filterModule)
		return filterModule->onClientPacket(packet, serverType);
	else
		return IFilter::onClientPacket(packet, serverType);
}

