#ifndef FILTERPROXY_H
#define FILTERPROXY_H

#include "IFilter.h"
#include "Core/Object.h"

class FilterManager;

class FilterProxy : public IFilter
{
public:
	FilterProxy(FilterManager* filterManager);
	virtual ~FilterProxy();
	virtual bool onServerPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet, ServerType serverType);
	virtual bool onClientPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet, ServerType serverType);


protected:
	void setFilterModule(IFilter* filterModule) { this->filterModule = filterModule; }
	IFilter* getFilterModule() { return filterModule; }
	friend class FilterManager;

private:
	FilterManager* filterManager;
	IFilter* filterModule;
};

#endif // FILTERPROXY_H
