#ifndef FILTERPROXY_H
#define FILTERPROXY_H

#include "Core/Object.h"
#include "IFilter.h"

class FilterManager;

class FilterProxy : public IFilter {
public:
	FilterProxy(FilterManager* filterManager, IFilterEndpoint* client, IFilterEndpoint* server, ServerType serverType);
	virtual ~FilterProxy();
	virtual bool onServerPacket(const TS_MESSAGE* packet);
	virtual bool onClientPacket(const TS_MESSAGE* packet);

protected:
	void recreateFilterModule(CreateFilterFunction createFilterFunction, DestroyFilterFunction destroyFilterFunction);

	friend class FilterManager;

private:
	FilterManager* filterManager;
	IFilter* filterModule;
};

#endif  // FILTERPROXY_H
