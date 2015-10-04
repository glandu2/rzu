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
	ref->declareLoaderAndLoad(ref->awakenOptionSidBinding);
	ref->declareLoaderAndLoad(ref->farmSidBinding);
	ref->declareLoaderAndLoad(ref->itemSidBinding);
	ref->declareLoaderAndLoad(ref->petSidBinding);
	ref->declareLoaderAndLoad(ref->skillSidBinding);
	ref->declareLoaderAndLoad(ref->summonSidBinding);
	ref->declareLoaderAndLoad(ref->titleSidBinding);
	ref->declareLoaderAndLoad(ref->titleConditionSidBinding);
}

bool ReferenceDataMgr::isWordBanned(const std::string &word) {
	auto& bannedWords = banWordResource.getData();
	return bannedWords.find(word) != bannedWords.end();
}

uint64_t ReferenceDataMgr::allocateAwakenOptionSid() {
	return awakenOptionSidBinding.getNextSid();
}

uint64_t ReferenceDataMgr::allocateFarmSid() {
	return farmSidBinding.getNextSid();
}

uint64_t ReferenceDataMgr::allocateItemSid() {
	return itemSidBinding.getNextSid();
}

uint64_t ReferenceDataMgr::allocatePetSid() {
	return petSidBinding.getNextSid();
}

uint64_t ReferenceDataMgr::allocateSkillSid() {
	return skillSidBinding.getNextSid();
}

uint64_t ReferenceDataMgr::allocateSummonSid() {
	return summonSidBinding.getNextSid();
}

uint64_t ReferenceDataMgr::allocateTitleSid() {
	return titleSidBinding.getNextSid();
}

uint64_t ReferenceDataMgr::allocateTitleConditionSid() {
	return titleConditionSidBinding.getNextSid();
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
