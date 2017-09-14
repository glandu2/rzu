#ifndef FILTERMANAGER_H
#define FILTERMANAGER_H

#include "IFilter.h"
#include "Core/Object.h"
#include "uv.h"
#include "Core/Timer.h"
#include <list>
#include <memory>

class FilterProxy;

class FilterManager : public Object
{
	DECLARE_CLASSNAME(FilterManager, 0)
private:
	typedef void (*DestroyFilterFunction)(IFilter* filter);
	typedef IFilter* (*CreateFilterFunction)(IFilterEndpoint* client, IFilterEndpoint* server, IFilter* oldFilter);

public:
	FilterManager();
	~FilterManager();

	static FilterManager* getInstance();

	FilterProxy* createFilter(IFilterEndpoint* client, IFilterEndpoint* server);
	void destroyFilter(FilterProxy* filter);

protected:
	void destroyInternalFilter(IFilter* filterModule);
	friend class FilterProxy;

protected:
	bool unloadModule();
	void loadModule();
	void onUpdateFilter();
	static void onFsStatDone(uv_fs_t* req);
	static void onFsRemoveDone(uv_fs_t* req);
	static void onFsMoveDone(uv_fs_t* req);

private:
	uv_lib_t filterModule;
	bool filterModuleLoaded;
	std::list<std::unique_ptr<FilterProxy> > packetFilters;
	CreateFilterFunction createFilterFunction;
	DestroyFilterFunction destroyFilterFunction;

	Timer<FilterManager> updateFilterTimer;
	int lastFileSize;
};

#endif // FILTERMANAGER_H
