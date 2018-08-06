#include "FilterManager.h"
#include "Console/ConsoleCommands.h"
#include "Core/EventLoop.h"
#include "FilterProxy.h"
#include "GlobalConfig.h"
#include "uv.h"
#include <memory>
#include <time.h>

#ifdef _WIN32
static const char* moduleSuffix = ".dll";
#elif defined(__APPLE__)
#include <unistd.h>
static const char* moduleSuffix = ".dylib";
#else
#include <unistd.h>
static const char* moduleSuffix = ".so";
#endif

std::list<FilterManager*> FilterManager::instance;

FilterManager::FilterManager(cval<std::string>& filterModuleName)
    : filterModuleName(filterModuleName),
      filterModuleLoaded(false),
      currentUsedIndex(0),
      createFilterFunction(nullptr),
      destroyFilterFunction(nullptr),
      lastFileSize(-1) {
	onUpdateFilter();

	updateFilterTimer.unref();
	updateFilterTimer.start(this, &FilterManager::onUpdateFilter, 2000, 2000);

	instance.push_back(this);
}

FilterManager::~FilterManager() {
	instance.remove(this);
	packetFilters.clear();
	unloadModule();
}

void FilterManager::init() {
	ConsoleCommands::get()->addCommand(
	    "filter.reload", "freload", 0, 0, &reloadFiltersCommand, "Reload filter for all connections");
}

std::unique_ptr<FilterProxy> FilterManager::createFilter(IFilter::ServerType serverType) {
	std::unique_ptr<FilterProxy> filterProxy = std::make_unique<FilterProxy>(this, serverType);

	packetFilters.push_back(filterProxy.get());

	return filterProxy;
}

std::string FilterManager::getName() {
	return filterModuleName.get();
}

void FilterManager::reloadFilter(FilterProxy* filterProxy) {
	if(filterModuleLoaded) {
		filterProxy->recreateFilterModule(createFilterFunction, destroyFilterFunction);
	}
}

void FilterManager::unregisterFilter(FilterProxy* filter) {
	packetFilters.remove(filter);
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

void FilterManager::reloadAllFilters(IFilter::CreateFilterFunction createFilterFunction,
                                     IFilter::DestroyFilterFunction destroyOldFilterFunction) {
	for(FilterProxy* filterProxy : packetFilters) {
		filterProxy->recreateFilterModule(createFilterFunction, destroyOldFilterFunction);
	}
}

int FilterManager::getNextUsedIndex() {
	return !currentUsedIndex;
}

std::string FilterManager::getUsedModuleName(int usedIndex) {
	return filterModuleName.get() + std::string("_used.") + std::to_string(usedIndex) + moduleSuffix;
}

void FilterManager::findUsedIndexOnDisk() {
	uv_fs_t statReq;
	uv_fs_stat(EventLoop::getLoop(), &statReq, getUsedModuleName(currentUsedIndex).c_str(), nullptr);
	uv_fs_req_cleanup(&statReq);
	if(statReq.result != 0)
		currentUsedIndex = !currentUsedIndex;
}

void FilterManager::reloadFiltersCommand(IWritableConsole* console, const std::vector<std::string>&) {
	for(FilterManager* self : instance) {
		self->reloadAllFilters(self->createFilterFunction, self->destroyFilterFunction);
		std::string filterName = self->filterModuleName.get();
		console->writef(
		    "Filter %s reloaded, %zu connection affected\r\n", filterName.c_str(), self->packetFilters.size());
	}
}

void FilterManager::loadModule() {
	uv_lib_t filterModule;
	IFilter::CreateFilterFunction createFilterFunction;
	IFilter::DestroyFilterFunction destroyFilterFunction;

	int err;
	const char* moduleName;

	std::string newModuleName = filterModuleName.get() + moduleSuffix;
	std::string oldUsedModuleName = getUsedModuleName(currentUsedIndex);

	int fileSize = getFileSize(newModuleName.c_str());

	if(fileSize > 0 && (fileSize == lastFileSize || lastFileSize == -1)) {
		moduleName = newModuleName.c_str();
		lastFileSize = fileSize;
	} else if(!filterModuleLoaded && fileSize == 0) {
		findUsedIndexOnDisk();
		oldUsedModuleName = getUsedModuleName(currentUsedIndex);
		moduleName = oldUsedModuleName.c_str();
	} else {
		if(fileSize > 0)
			log(LL_Debug, "Module size was modified, will reload next time\n");
		lastFileSize = fileSize;
		return;
	}

	std::string usedModuleName =
	    filterModuleName.get() + std::string("_used.") + std::to_string(getNextUsedIndex()) + moduleSuffix;

	log(LL_Info, "Loading filter module %s\n", moduleName);

	err = rename(moduleName, usedModuleName.c_str());
	if(err) {
		log(LL_Error,
		    "Failed to rename %s to %s: %s (%d)\n",
		    moduleName,
		    usedModuleName.c_str(),
		    strerror(errno),
		    errno);
		return;
	}

	err = uv_dlopen(usedModuleName.c_str(), &filterModule);
	if(err) {
		log(LL_Error, "Can't load filter module: %s (%d)\n", uv_strerror(err), err);
		return;
	}

	err = uv_dlsym(&filterModule, "createFilter", (void**) &createFilterFunction);
	if(err) {
		log(LL_Error,
		    "Can't find function createFilter in library %s: %s (%d)\n",
		    usedModuleName.c_str(),
		    uv_strerror(err),
		    err);
		uv_dlclose(&filterModule);
		return;
	}
	err = uv_dlsym(&filterModule, "destroyFilter", (void**) &destroyFilterFunction);
	if(err) {
		log(LL_Error,
		    "Can't find function destroyFilter in library %s: %s (%d)\n",
		    usedModuleName.c_str(),
		    uv_strerror(err),
		    err);
		uv_dlclose(&filterModule);
		return;
	}

	reloadAllFilters(createFilterFunction, this->destroyFilterFunction);

	unloadModule();
	unlink(oldUsedModuleName.c_str());

	this->filterModule = filterModule;
	this->filterModuleLoaded = true;
	this->currentUsedIndex = getNextUsedIndex();
	this->createFilterFunction = createFilterFunction;
	this->destroyFilterFunction = destroyFilterFunction;
	log(LL_Info, "Loaded new filter %s\n", newModuleName.c_str());
}

void FilterManager::onUpdateFilter() {
	loadModule();
}
