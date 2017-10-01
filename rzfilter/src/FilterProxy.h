#ifndef FILTERPROXY_H
#define FILTERPROXY_H

#include "Core/Object.h"
#include "IFilter.h"

class FilterManager;

class FilterProxy : public IFilter {
public:
	FilterProxy(FilterManager* filterManager, IFilterEndpoint* client, IFilterEndpoint* server);
	virtual ~FilterProxy();
	virtual bool onServerPacket(const TS_MESSAGE* packet, ServerType serverType);
	virtual bool onClientPacket(const TS_MESSAGE* packet, ServerType serverType);

protected:
	void setFilterModule(IFilter* filterModule) { this->filterModule = filterModule; }
	IFilter* getFilterModule() { return filterModule; }
	IFilterEndpoint* getClientEndpoint() { return client; }
	IFilterEndpoint* getServerEndpoint() { return server; }

	friend class FilterManager;

private:
	FilterManager* filterManager;
	IFilter* filterModule;
};

#endif  // FILTERPROXY_H
