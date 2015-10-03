#include "BannedWords.h"
#include "../../GlobalConfig.h"
#include <iterator>

DECLARE_DB_BINDING(GameServer::BannedWordsBinding, "banwords");
template<> void DbQueryJob<GameServer::BannedWordsBinding>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
				  CONFIG_GET()->game.arcadia.connectionString,
				  "select * from BanWordResource",
				  DbQueryBinding::EM_MultiRows);

	addColumn("string", &OutputType::word);
}

namespace GameServer {

void BannedWordsBinding::load() {
	dbQuery.executeDbQuery<BannedWordsBinding>(this, &BannedWordsBinding::onDataLoaded, BannedWordsBinding::Input());
}

void BannedWordsBinding::onDataLoaded(DbQueryJob<BannedWordsBinding>* query) {
	auto& results = query->getResults();
	data.rehash(results.size());

	auto it = results.begin();
	auto itEnd = results.end();
	for(; it != itEnd; ++it) {
		const std::unique_ptr<BannedWords>& bannedWord = *it;
		auto insertResult = data.insert(bannedWord.get()->word);
		if(insertResult.second == false) {
			log(LL_Warning, "duplicate word \"%s\"\n", insertResult.first->c_str());
		}
	}

	log(LL_Info, "Loaded %d lines\n", (int)data.size());

	dataLoaded();
}

}
