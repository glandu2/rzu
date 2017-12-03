#ifndef FILTERMANAGER_H
#define FILTERMANAGER_H

#include "Core/Object.h"
#include "Core/Timer.h"
#include "IFilter.h"
#include "uv.h"
#include <list>
#include <memory>

class FilterProxy;
class IWritableConsole;

class FilterManager : public Object {
	DECLARE_CLASSNAME(FilterManager, 0)

public:
	static FilterManager* getInstance();

	FilterProxy* createFilter(IFilterEndpoint* client, IFilterEndpoint* server, IFilter::ServerType serverType);
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
	void reloadAllFilters(IFilter::CreateFilterFunction createFilterFunction,
	                      IFilter::DestroyFilterFunction destroyOldFilterFunction);

	static void reloadFiltersCommand(IWritableConsole* console, const std::vector<std::string>&);

private:
	uv_lib_t filterModule;
	bool filterModuleLoaded;
	std::list<std::unique_ptr<FilterProxy> > packetFilters;
	IFilter::CreateFilterFunction createFilterFunction;
	IFilter::DestroyFilterFunction destroyFilterFunction;

	Timer<FilterManager> updateFilterTimer;
	int lastFileSize;

	FilterManager();
	~FilterManager();
};

#endif  // FILTERMANAGER_H
