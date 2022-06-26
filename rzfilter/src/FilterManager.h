#pragma once

#include "Config/ConfigParamVal.h"
#include "Core/Object.h"
#include "Core/Timer.h"
#include "IFilter.h"
#include "uv.h"
#include <list>
#include <memory>

class FilterProxy;
class IWritableConsole;

class FilterManager : public Object {
	DECLARE_CLASS(FilterManager)

public:
	FilterManager(cval<std::string>& filterModuleName);
	~FilterManager();

	static void init();

	std::unique_ptr<FilterProxy> createFilter(SessionType sessionType);
	std::string getName();

protected:
	void unregisterFilter(FilterProxy* filter);
	void destroyInternalFilter(IFilter* filterModule);
	void reloadFilter(FilterProxy* filterProxy);
	friend class FilterProxy;

protected:
	bool unloadModule();
	void loadModule();
	void onUpdateFilter();
	void reloadAllFilters(IFilter::CreateFilterFunction createFilterFunction,
	                      IFilter::DestroyFilterFunction destroyOldFilterFunction);

	int getNextUsedIndex();
	std::string getUsedModuleName(int usedIndex);
	void findUsedIndexOnDisk();

	static void reloadFiltersCommand(IWritableConsole* console, const std::vector<std::string>&);

private:
	cval<std::string>& filterModuleName;
	uv_lib_t filterModule;
	void* filterModuleArgument;
	bool filterModuleLoaded;
	int currentUsedIndex;
	std::list<FilterProxy*> packetFilters;
	IFilter::CreateFilterFunction createFilterFunction;
	IFilter::DestroyFilterFunction destroyFilterFunction;
	IFilter::DestroyGlobalFilterFunction destroyGlobalFilterFunction;

	Timer<FilterManager> updateFilterTimer;
	int lastFileSize;

	static std::list<FilterManager*> instance;
};
