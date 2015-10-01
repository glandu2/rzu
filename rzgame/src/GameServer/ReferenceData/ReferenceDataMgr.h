#ifndef REFERENCEDATAMGR_H
#define REFERENCEDATAMGR_H

#include "Core/Object.h"
#include "BannedWords.h"

namespace GameServer {

class RefDataLoader;

class ReferenceDataMgr : public Object {
	DECLARE_CLASS(GameServer::ReferenceDataMgr)
public:
	typedef void (*OnReferenceDataLoaded)(void* data);
public:
	static ReferenceDataMgr* get();
	static void load(OnReferenceDataLoaded callback, void* data);

	bool isWordBanned(const std::string& word);

protected:
	friend class RefDataLoader;
	void signalDataLoaded(RefDataLoader* loader);

	void declareLoaderAndLoad(RefDataLoader& loader);
private:
	OnReferenceDataLoaded loadedCallback;
	void* data;

	std::vector<RefDataLoader*> pendingLoaders;

	BannedWordsBinding banWordResource;
};

}

#endif // REFERENCEDATAMGR_H
