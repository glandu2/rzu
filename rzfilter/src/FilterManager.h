#ifndef FILTERMANAGER_H
#define FILTERMANAGER_H

#include "IFilter.h"
#include "Core/Object.h"
#include "uv.h"
#include "Core/Timer.h"

class FilterManager : public Object, public IFilter
{
	DECLARE_CLASSNAME(FilterManager, 0)
private:
	typedef void (*DestroyFilterFunction)(IFilter* filter);

public:
	FilterManager();
	virtual ~FilterManager();
	virtual bool onServerPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet);
	virtual bool onClientPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet);

protected:
	bool unloadModule();
	void loadModule();
	void onUpdateFilter();
	static void onFsStatDone(uv_fs_t* req);
	static void onFsRemoveDone(uv_fs_t* req);
	static void onFsMoveDone(uv_fs_t* req);

private:
	uv_lib_t filterModule;
	IFilter* packetFilter;
	DestroyFilterFunction destroyFilterFunction;

	Timer<FilterManager> updateFilterTimer;
	int lastFileSize;
};

#endif // FILTERMANAGER_H
