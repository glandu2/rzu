#include "BannedWords.h"
#include "../../GlobalConfig.h"
#include <iterator>

template<>
DbQueryBinding* DbQueryJob<GameServer::BannedWordsBinding>::dbBinding = nullptr;

template<>
const char* DbQueryJob<GameServer::BannedWordsBinding>::SQL_CONFIG_NAME = "banwords";

template<>
bool DbQueryJob<GameServer::BannedWordsBinding>::init(DbConnectionPool* dbConnectionPool) {
	std::vector<DbQueryBinding::ParameterBinding> params;
	std::vector<DbQueryBinding::ColumnBinding> cols;

	addColumn(cols, "string", &OutputType::word);

	createBinding(dbConnectionPool,
				  CONFIG_GET()->game.arcadia.connectionString,
				  "select * from BanWordResource",
				  params,
				  cols,
				  DbQueryBinding::EM_MultiRows);

	return true;
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
		} else {
			log(LL_Debug, "Loaded banned word \"%s\"\n", insertResult.first->c_str());
		}
	}

	log(LL_Info, "Loaded %d lines\n", (int)data.size());

	dataLoaded();
}

}
