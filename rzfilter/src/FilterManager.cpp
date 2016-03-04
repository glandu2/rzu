#include "FilterManager.h"
#include "uv.h"
#include "Core/EventLoop.h"
#include <time.h>

#ifdef _WIN32
	static const char* newModuleName = "rzfilter_module.dll";
	static const char* usedModuleName = "rzfilter_module_used.dll";
#elif defined(__APPLE__)
	static const char* newModuleName = "rzfilter_module.dylib";
	static const char* usedModuleName = "rzfilter_module_used.dylib";
#else
	#include <unistd.h>
	static const char* newModuleName = "rzfilter_module.so";
	static const char* usedModuleName = "rzfilter_module_used.so";
#endif

FilterManager::FilterManager() : packetFilter(nullptr), destroyFilterFunction(nullptr), lastFileSize(0) {
	onUpdateFilter();

	updateFilterTimer.start(this, &FilterManager::onUpdateFilter, 2000, 2000);
}

FilterManager::~FilterManager()
{
	unloadModule();
}

bool FilterManager::onServerPacket(IFilterEndpoint *client, IFilterEndpoint *server, const TS_MESSAGE *packet)
{
	if(packetFilter)
		return packetFilter->onServerPacket(client, server, packet);
	else
		return IFilter::onServerPacket(client, server, packet);
}

bool FilterManager::onClientPacket(IFilterEndpoint *client, IFilterEndpoint *server, const TS_MESSAGE *packet)
{
	if(packetFilter)
		return packetFilter->onClientPacket(client, server, packet);
	else
		return IFilter::onClientPacket(client, server, packet);
}

bool FilterManager::unloadModule() {
	if(packetFilter && destroyFilterFunction) {
		log(LL_Info, "Unloading filter module %p\n", packetFilter);
		destroyFilterFunction(packetFilter);
		uv_dlclose(&filterModule);
		packetFilter = nullptr;
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

void FilterManager::loadModule()
{
	uv_lib_t filterModule;
	IFilter* newPacketFilter;
	IFilter* (*createFilterFunction)(IFilter*);
	void (*destroyFilterFunction)(IFilter*);

	int err;
	const char* moduleName;
	char generatedModuleName[128];

	int fileSize = getFileSize(newModuleName);

	if(fileSize > 0 && fileSize == lastFileSize) {
		moduleName = newModuleName;
	} else if(this->packetFilter == nullptr && fileSize == 0) {
		moduleName = usedModuleName;
	} else {
		if(fileSize > 0)
			log(LL_Debug, "Module size was modified, will reload next time\n");
		lastFileSize = fileSize;
		return;
	}

	log(LL_Info, "Loading filter module %s\n", moduleName);

	sprintf(generatedModuleName, "%s.%d", newModuleName, (int)time(NULL));

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

	err = uv_dlsym(&filterModule, "createFilter", (void**)&createFilterFunction);
	if(err) {
		log(LL_Error, "Can't find function createFilter in library %s: %s (%d)\n", generatedModuleName, uv_strerror(err), err);
		uv_dlclose(&filterModule);
		return;
	}
	err = uv_dlsym(&filterModule, "destroyFilter", (void**)&destroyFilterFunction);
	if(err) {
		log(LL_Error, "Can't find function destroyFilter in library %s: %s (%d)\n", generatedModuleName, uv_strerror(err), err);
		uv_dlclose(&filterModule);
		return;
	}

	newPacketFilter = createFilterFunction(this->packetFilter);

	unloadModule();
	unlink(usedModuleName);
	err = rename(generatedModuleName, usedModuleName);
	if(err) {
		log(LL_Error, "Failed to rename %s to %s: %s (%d)\n", generatedModuleName, usedModuleName, strerror(errno), errno);
	}

	this->filterModule = filterModule;
	this->packetFilter = newPacketFilter;
	this->destroyFilterFunction = destroyFilterFunction;
	log(LL_Info, "Loaded new filter %p from %s\n", newPacketFilter, newModuleName);
}

void FilterManager::onUpdateFilter() {
	loadModule();
}
