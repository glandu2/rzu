#ifndef REFDATALOADER_H
#define REFDATALOADER_H

#include "Core/Object.h"
#include "Database/DbQueryJobCallback.h"

namespace GameServer {

class ReferenceDataMgr;

class RefDataLoader : public Object
{
public:
	RefDataLoader();

	void loadData(ReferenceDataMgr* refMgr);
	virtual void load() = 0;

protected:
	void dataLoaded();

private:
	ReferenceDataMgr* refMgr;
};

template<class RefDataStruct, class RefDataBinding>
class RefDataLoaderHelper : public RefDataLoader
{
public:
	struct Input {};
	typedef RefDataStruct Output;

public:
	void load() {
		dbQuery.executeDbQuery<RefDataBinding>(this, &RefDataLoaderHelper::onDataLoaded, RefDataLoaderHelper::Input());
	}

	const RefDataStruct* getData(int32_t id) {
		auto it = data.find(id);
		if(it != data.end())
			return it->second.get();
		return nullptr;
	}

protected:
	void onDataLoaded(DbQueryJob<RefDataBinding>* query) {
		auto& results = query->getResults();
		if(data.empty())
			data.rehash(results.size());

		auto it = results.begin();
		auto itEnd = results.end();
		for(; it != itEnd; ++it) {
			std::unique_ptr<RefDataStruct>& line = *it;
			auto it = data.find(line->id);
			if(it == data.end()) {
				data.insert(std::make_pair(line->id, std::move(line)));
			} else {
				*(it->second) = *line;
			}
		}

		log(LL_Info, "Loaded %d lines\n", (int)data.size());

		dataLoaded();
	}

private:
	DbQueryJobRef dbQuery;
	std::unordered_map<int32_t, const std::unique_ptr<RefDataStruct>> data;
};

}

#endif // REFDATALOADER_H