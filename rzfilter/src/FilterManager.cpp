#include "FilterManager.h"
#include "Core/EventLoop.h"
#include "FilterProxy.h"
#include "GlobalConfig.h"
#include "uv.h"
#include <time.h>

#ifdef _WIN32
static const char* moduleSuffix = ".dll";
static const char* usedModuleSuffix = "_used.dll";
#elif defined(__APPLE__)
#include <unistd.h>
static const char* moduleSuffix = ".dylib";
static const char* usedModuleSuffix = "_used.dylib";
#else
#include <unistd.h>
static const char* moduleSuffix = ".so";
static const char* usedModuleSuffix = "_used.so";
#endif

FilterManager::FilterManager()
    : filterModuleLoaded(false), createFilterFunction(nullptr), destroyFilterFunction(nullptr), lastFileSize(-1) {
	onUpdateFilter();

	updateFilterTimer.unref();
	updateFilterTimer.start(this, &FilterManager::onUpdateFilter, 2000, 2000);
}

FilterManager::~FilterManager() {
	auto it = packetFilters.begin();
	auto itEnd = packetFilters.end();
	for(; it != itEnd;) {
		it = packetFilters.erase(it);
	}
	unloadModule();
}

FilterManager* FilterManager::getInstance() {
	static FilterManager filterManager;
	return &filterManager;
}

FilterProxy* FilterManager::createFilter(IFilterEndpoint* client, IFilterEndpoint* server) {
	FilterProxy* filterProxy = new FilterProxy(this, client, server);
	if(filterModuleLoaded) {
		filterProxy->setFilterModule(createFilterFunction(client, server, nullptr));
	}

	packetFilters.push_back(std::unique_ptr<FilterProxy>(filterProxy));

	return filterProxy;
}

void FilterManager::destroyFilter(FilterProxy* filter) {
	auto it = packetFilters.begin();
	auto itEnd = packetFilters.end();
	for(; it != itEnd; ++it) {
		if(it->get() == filter) {
			packetFilters.erase(it);
			break;
		}
	}
}

void FilterManager::destroyInternalFilter(IFilter* filterModule) {
	if(filterModuleLoaded) {
		destroyFilterFunction(filterModule);
	}
}

bool FilterManager::unloadModule() {
	if(filterModuleLoaded) {
		log(LL_Info, "Unloading filter module\n");
		uv_dlclose(&filterModule);
		filterModuleLoaded = false;
		createFilterFunction = nullptr;
		destroyFilterFunction = nullptr;
		return true;
	}

	return false;
}

static int getFileSize(const char* name) {
	struct stat info;
	int ret = stat(name, &info);
	if(ret == 0 && (info.st_mode & S_IFREG))
		return info.st_size;
	else
		return 0;
}

void FilterManager::loadModule() {
	uv_lib_t filterModule;
	CreateFilterFunction createFilterFunction;
	DestroyFilterFunction destroyFilterFunction;

	int err;
	const char* moduleName;
	char generatedModuleName[128];

	std::string newModuleName = CONFIG_GET()->filter.filterModuleName.get() + moduleSuffix;
	std::string usedModuleName = CONFIG_GET()->filter.filterModuleName.get() + usedModuleSuffix;

	int fileSize = getFileSize(newModuleName.c_str());

	if(fileSize > 0 && (fileSize == lastFileSize || lastFileSize == -1)) {
		moduleName = newModuleName.c_str();
		lastFileSize = fileSize;
	} else if(!filterModuleLoaded && fileSize == 0) {
		moduleName = usedModuleName.c_str();
	} else {
		if(fileSize > 0)
			log(LL_Debug, "Module size was modified, will reload next time\n");
		lastFileSize = fileSize;
		return;
	}

	log(LL_Info, "Loading filter module %s\n", moduleName);

	sprintf(generatedModuleName, "%s.%d", newModuleName.c_str(), (int) time(NULL));

	err = rename(moduleName, generatedModuleName);
	if(err) {
		log(LL_Error, "Failed to rename %s to %s: %s (%d)\n", moduleName, generatedModuleName, strerror(errno), errno);
		return;
	}

	err = uv_dlopen(generatedModuleName, &filterModule);
	if(err) {
		log(LL_Error, "Can't load filter module: %s (%d)\n", uv_strerror(err), err);
		return;
	}

	err = uv_dlsym(&filterModule, "createFilter", (void**) &createFilterFunction);
	if(err) {
		log(LL_Error,
		    "Can't find function createFilter in library %s: %s (%d)\n",
		    generatedModuleName,
		    uv_strerror(err),
		    err);
		uv_dlclose(&filterModule);
		return;
	}
	err = uv_dlsym(&filterModule, "destroyFilter", (void**) &destroyFilterFunction);
	if(err) {
		log(LL_Error,
		    "Can't find function destroyFilter in library %s: %s (%d)\n",
		    generatedModuleName,
		    uv_strerror(err),
		    err);
		uv_dlclose(&filterModule);
		return;
	}

	auto it = packetFilters.begin();
	auto itEnd = packetFilters.end();
	for(; it != itEnd; ++it) {
		FilterProxy* filterProxy = it->get();
		IFilter* oldFilter = filterProxy->getFilterModule();
		filterProxy->setFilterModule(
		    createFilterFunction(filterProxy->getClientEndpoint(), filterProxy->getServerEndpoint(), oldFilter));
		if(oldFilter && this->destroyFilterFunction)
			this->destroyFilterFunction(oldFilter);
	}

	unloadModule();
	unlink(usedModuleName.c_str());
	err = rename(generatedModuleName, usedModuleName.c_str());
	if(err) {
		log(LL_Error,
		    "Failed to rename %s to %s: %s (%d)\n",
		    generatedModuleName,
		    usedModuleName.c_str(),
		    strerror(errno),
		    errno);
	}

	this->filterModule = filterModule;
	this->filterModuleLoaded = true;
	this->createFilterFunction = createFilterFunction;
	this->destroyFilterFunction = destroyFilterFunction;
	log(LL_Info, "Loaded new filter %s\n", newModuleName.c_str());
}

void FilterManager::onUpdateFilter() {
	loadModule();
}
