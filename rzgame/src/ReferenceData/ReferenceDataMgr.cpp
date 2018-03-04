#include "ReferenceDataMgr.h"
#include "Console/ConsoleCommands.h"
#include "RefDataLoader.h"
#include <algorithm>

namespace GameServer {

ReferenceDataMgr* ReferenceDataMgr::get() {
	static ReferenceDataMgr referenceData;
	return &referenceData;
}

ReferenceDataMgr::ReferenceDataMgr() {
	loaders.push_back(&banWordResource);
	loaders.push_back(&jobResource);
	loaders.push_back(&statResource);
	loaders.push_back(&jobLevelBonus);
	loaders.push_back(&itemResource);

	loaders.push_back(&awakenOptionSidBinding);
	loaders.push_back(&farmSidBinding);
	loaders.push_back(&itemSidBinding);
	loaders.push_back(&petSidBinding);
	loaders.push_back(&skillSidBinding);
	loaders.push_back(&summonSidBinding);
	loaders.push_back(&titleSidBinding);
	loaders.push_back(&titleConditionSidBinding);

	ConsoleCommands::get()->addCommand(
	    "game.reload", "reload", 0, 0, &commandReload, "Reload Arcadia", "reload : reload reference data from arcadia");
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

game_sid_t ReferenceDataMgr::allocateAwakenOptionSid() {
	return awakenOptionSidBinding.getNextSid();
}

game_sid_t ReferenceDataMgr::allocateFarmSid() {
	return farmSidBinding.getNextSid();
}

game_sid_t ReferenceDataMgr::allocateItemSid() {
	return itemSidBinding.getNextSid();
}

game_sid_t ReferenceDataMgr::allocatePetSid() {
	return petSidBinding.getNextSid();
}

game_sid_t ReferenceDataMgr::allocateSkillSid() {
	return skillSidBinding.getNextSid();
}

game_sid_t ReferenceDataMgr::allocateSummonSid() {
	return summonSidBinding.getNextSid();
}

game_sid_t ReferenceDataMgr::allocateTitleSid() {
	return titleSidBinding.getNextSid();
}

game_sid_t ReferenceDataMgr::allocateTitleConditionSid() {
	return titleConditionSidBinding.getNextSid();
}

void ReferenceDataMgr::signalDataLoaded(RefDataLoader* loader) {
	state.remaining--;

	if(state.remaining <= 0) {
		state.inProgress = false;
		if(state.loadedCallback)
			(*state.loadedCallback)(state.data);
	}
}

void ReferenceDataMgr::commandReload(IWritableConsole* console, const std::vector<std::string>& args) {
	ReferenceDataMgr::get()->load(nullptr, nullptr);
}

}  // namespace GameServer
