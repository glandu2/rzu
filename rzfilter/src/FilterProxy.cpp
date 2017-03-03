#include "FilterProxy.h"
#include "FilterManager.h"

FilterProxy::FilterProxy(FilterManager* filterManager) : filterManager(filterManager), filterModule(nullptr)
{
}

FilterProxy::~FilterProxy()
{
	if(filterModule)
		filterManager->destroyInternalFilter(filterModule);
}

bool FilterProxy::onServerPacket(IFilterEndpoint *client, IFilterEndpoint *server, const TS_MESSAGE *packet, ServerType serverType)
{
	if(filterModule)
		return filterModule->onServerPacket(client, server, packet, serverType);
	else
		return IFilter::onServerPacket(client, server, packet, serverType);
}

bool FilterProxy::onClientPacket(IFilterEndpoint *client, IFilterEndpoint *server, const TS_MESSAGE *packet, ServerType serverType)
{
	if(filterModule)
		return filterModule->onClientPacket(client, server, packet, serverType);
	else
		return IFilter::onClientPacket(client, server, packet, serverType);
}

