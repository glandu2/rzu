#include "ReferenceDataMgr.h"
#include "RefDataLoader.h"
#include <algorithm>
#include "Console/ConsoleCommands.h"

namespace GameServer {

ReferenceDataMgr *ReferenceDataMgr::get() {
	static ReferenceDataMgr referenceData;
	return &referenceData;
}

ReferenceDataMgr::ReferenceDataMgr() {
	loaders.push_back(&banWordResource);
	loaders.push_back(&awakenOptionSidBinding);
	loaders.push_back(&farmSidBinding);
	loaders.push_back(&itemSidBinding);
	loaders.push_back(&petSidBinding);
	loaders.push_back(&skillSidBinding);
	loaders.push_back(&summonSidBinding);
	loaders.push_back(&titleSidBinding);
	loaders.push_back(&titleConditionSidBinding);

	ConsoleCommands::get()->addCommand("game.reload", "reload", 0, &commandReload,
									   "Reload Arcadia",
									   "reload : reload reference data from arcadia");
}

void ReferenceDataMgr::load(OnReferenceDataLoaded callback, void* data) {
	ReferenceDataMgr* ref = get();

	if(ref->state.inProgress) {
		ref->log(LL_Error, "Load request while already loading data\n");
	} else {
		ref->state.inProgress = true;
		ref->state.loadedCallback = callback;
		ref->state.data = data;
		ref->state.remaining = ref->loaders.size();

		auto it = ref->loaders.begin();
		auto itEnd = ref->loaders.end();
		for(; it != itEnd; ++it) {
			RefDataLoader* loaderCallback = *it;
			loaderCallback->loadData(ref);
		}
	}
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
	state.remaining--;

	if(state.remaining <= 0) {
		state.inProgress = false;
		if(state.loadedCallback)
			(*state.loadedCallback)(state.data);
	}
}

void ReferenceDataMgr::commandReload(IWritableConsole *console, const std::vector<std::string> &args) {
	ReferenceDataMgr::get()->load(nullptr, nullptr);
}

}
