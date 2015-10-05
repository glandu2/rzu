#ifndef REFERENCEDATAMGR_H
#define REFERENCEDATAMGR_H

#include "Core/Object.h"
#include "BannedWords.h"
#include "ObjectSidState.h"

namespace GameServer {

class RefDataLoader;

class ReferenceDataMgr : public Object {
	DECLARE_CLASS(GameServer::ReferenceDataMgr)
public:
	typedef void (*OnReferenceDataLoaded)(void* data);
public:
	static ReferenceDataMgr* get();
	static void load(OnReferenceDataLoaded callback, void* data);

	ReferenceDataMgr();

	bool isWordBanned(const std::string& word);
	uint64_t allocateAwakenOptionSid();
	uint64_t allocateFarmSid();
	uint64_t allocateItemSid();
	uint64_t allocatePetSid();
	uint64_t allocateSkillSid();
	uint64_t allocateSummonSid();
	uint64_t allocateTitleSid();
	uint64_t allocateTitleConditionSid();

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
