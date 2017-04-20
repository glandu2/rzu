#ifndef BANNEDWORDS_H
#define BANNEDWORDS_H

#include "RefDataLoader.h"
#include "Database/DbQueryJobRef.h"
#include "Database/DbQueryJob.h"
#include <unordered_set>
#include <stdint.h>

namespace GameServer {

class BannedWords {
public:
	std::string word;
};

class BannedWordsBinding : public RefDataLoader {
	DECLARE_CLASS(GameServer::BannedWordsBinding)
public:
	struct Input {};

	typedef BannedWords Output;

	void load();
	static bool isWordBanned(const std::string& word);

protected:
	void onDataLoaded(DbQueryJob<BannedWordsBinding>* query);

private:
	DbQueryJobRef dbQuery;
	static std::unordered_set<std::string> data;
};

}

#endif // BANNEDWORDS_H
