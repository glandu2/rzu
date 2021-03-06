#include "BannedWords.h"
#include "Config/GlobalConfig.h"
#include <iterator>

template<> void DbQueryJob<GameServer::BannedWordsBinding>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
	              CONFIG_GET()->game.arcadia.connectionString,
	              "select * from BanWordResource",
	              DbQueryBinding::EM_MultiRows);

	addColumn("string", &OutputType::word);
}
DECLARE_DB_BINDING(GameServer::BannedWordsBinding, "banwords");

namespace GameServer {

std::unordered_set<std::string> BannedWordsBinding::data;

void BannedWordsBinding::load() {
	dbQuery.executeDbQuery<BannedWordsBinding>(this, &BannedWordsBinding::onDataLoaded, BannedWordsBinding::Input());
}

bool BannedWordsBinding::isWordBanned(const std::string& word) {
	return data.find(word) != data.end();
}

void BannedWordsBinding::onDataLoaded(DbQueryJob<BannedWordsBinding>* query) {
	auto& results = query->getResults();
	data.clear();
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

	log(LL_Info, "Loaded %d lines\n", (int) data.size());

	dataLoaded();
}

}  // namespace GameServer
