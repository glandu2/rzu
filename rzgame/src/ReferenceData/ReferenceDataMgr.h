#ifndef REFERENCEDATAMGR_H
#define REFERENCEDATAMGR_H

#include "Core/Object.h"
#include "ObjectSidState.h"
#include "BannedWords.h"
#include "JobResource.h"
#include "StatResource.h"
#include "JobLevelBonus.h"
#include "ItemResource.h"

namespace GameServer {

class RefDataLoader;
class StatBase;

class ReferenceDataMgr : public Object {
	DECLARE_CLASS(GameServer::ReferenceDataMgr)
public:
	typedef void (*OnReferenceDataLoaded)(void* data);
public:
	static ReferenceDataMgr* get();
	static void load(OnReferenceDataLoaded callback, void* data);

	ReferenceDataMgr();

	game_sid_t allocateAwakenOptionSid();
	game_sid_t allocateFarmSid();
	game_sid_t allocateItemSid();
	game_sid_t allocatePetSid();
	game_sid_t allocateSkillSid();
	game_sid_t allocateSummonSid();
	game_sid_t allocateTitleSid();
	game_sid_t allocateTitleConditionSid();

protected:
	friend class RefDataLoader;
	void signalDataLoaded(RefDataLoader* loader);

	void declareLoaderAndLoad(RefDataLoader& loader);

	static void commandReload(IWritableConsole* console, const std::vector<std::string>& args);

private:
	struct LoadingState {
		bool inProgress;
		OnReferenceDataLoaded loadedCallback;
		void* data;
		size_t remaining;
	};
	LoadingState state;

	std::vector<RefDataLoader*> loaders;

	BannedWordsBinding banWordResource;
	JobResourceBinding jobResource;
	StatResourceBinding statResource;
	JobLevelBonusBinding jobLevelBonus;
	ItemResourceBinding itemResource;


	AwakenOptionSidBinding awakenOptionSidBinding;
	FarmSidBinding farmSidBinding;
	ItemSidBinding itemSidBinding;
	PetSidBinding petSidBinding;
	SkillSidBinding skillSidBinding;
	SummonSidBinding summonSidBinding;
	TitleSidBinding titleSidBinding;
	TitleConditionSidBinding titleConditionSidBinding;
};

}

#endif // REFERENCEDATAMGR_H
