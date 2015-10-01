#include "ReferenceDataMgr.h"
#include "RefDataLoader.h"
#include <algorithm>

namespace GameServer {

ReferenceDataMgr *ReferenceDataMgr::get() {
	static ReferenceDataMgr referenceData;
	return &referenceData;
}

void ReferenceDataMgr::load(OnReferenceDataLoaded callback, void* data) {
	ReferenceDataMgr* ref = get();
	ref->loadedCallback = callback;
	ref->data = data;

	ref->declareLoaderAndLoad(ref->banWordResource);
}

bool ReferenceDataMgr::isWordBanned(const std::string &word) {
	auto& bannedWords = banWordResource.getData();
	return bannedWords.find(word) != bannedWords.end();
}

void ReferenceDataMgr::signalDataLoaded(RefDataLoader *loader) {
	auto it = std::find(pendingLoaders.begin(), pendingLoaders.end(), loader);
	if(it == pendingLoaders.end()) {
		log(LL_Fatal, "Can't find loader %s in pending loaders\n", loader->getObjectName());
	} else {
		pendingLoaders.erase(it);
	}

	if(pendingLoaders.empty())
		(*loadedCallback)(data);
}

void ReferenceDataMgr::declareLoaderAndLoad(RefDataLoader& loader) {
	pendingLoaders.push_back(&loader);
	loader.loadData(this);
}

}
