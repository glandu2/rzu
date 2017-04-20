#ifndef OBJECTSIDSTATE_H
#define OBJECTSIDSTATE_H

#include "RefDataLoader.h"
#include "Database/DbQueryJobRef.h"
#include "Database/DbQueryJob.h"
#include "GameTypes.h"
#include "Core/PrintfFormats.h"

namespace GameServer {

template<class T>
class ObjectSidStateBinding : public RefDataLoader {
public:
	struct Input {};

	struct Output {
		game_sid_t max_sid;
	};

	ObjectSidStateBinding() : loaded(false), next_sid(0) {}

	void load();
	game_sid_t getNextSid() { return next_sid++; }

protected:
	void onDataLoaded(DbQueryJob<T>* query);

private:
	DbQueryJobRef dbQuery;
	bool loaded;
	game_sid_t next_sid;
};

template<class T>
void ObjectSidStateBinding<T>::load() {
	if(loaded == false) {
		loaded = true;
		dbQuery.executeDbQuery<T>(this, &ObjectSidStateBinding::onDataLoaded, ObjectSidStateBinding::Input());
	} else {
		dataLoaded();
	}
}

template<class T>
void ObjectSidStateBinding<T>::onDataLoaded(DbQueryJob<T> *query) {
	auto& results = query->getResults();
	if(results.size() > 0)
		next_sid = results.front().get()->max_sid + 1;
	else
		next_sid = 0;

	log(LL_Info, "Next SID: %" PRIu64 "\n", next_sid);

	dataLoaded();
}

class AwakenOptionSidBinding : public ObjectSidStateBinding<AwakenOptionSidBinding> {
	DECLARE_CLASS(GameServer::AwakenOptionSidBinding)
};

class FarmSidBinding : public ObjectSidStateBinding<FarmSidBinding> {
	DECLARE_CLASS(GameServer::FarmSidBinding)
};

class ItemSidBinding : public ObjectSidStateBinding<ItemSidBinding> {
	DECLARE_CLASS(GameServer::ItemSidBinding)
};

class PetSidBinding : public ObjectSidStateBinding<PetSidBinding> {
	DECLARE_CLASS(GameServer::PetSidBinding)
};

class SkillSidBinding : public ObjectSidStateBinding<SkillSidBinding> {
	DECLARE_CLASS(GameServer::SkillSidBinding)
};

class SummonSidBinding : public ObjectSidStateBinding<SummonSidBinding> {
	DECLARE_CLASS(GameServer::SummonSidBinding)
};

class TitleSidBinding : public ObjectSidStateBinding<TitleSidBinding> {
	DECLARE_CLASS(GameServer::TitleSidBinding)
};

class TitleConditionSidBinding : public ObjectSidStateBinding<TitleConditionSidBinding> {
	DECLARE_CLASS(GameServer::TitleConditionSidBinding)
};


}

#endif // OBJECTSIDSTATE_H
