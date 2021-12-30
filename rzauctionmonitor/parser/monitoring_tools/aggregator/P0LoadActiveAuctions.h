#pragma once

#include "Core/Object.h"
#include "Database/DbQueryJobRef.h"
#include <functional>
#include <stdint.h>
#include <string>

struct AUCTION_FILE;

template<class T> class DbQueryJob;

struct LastParsedFileName {
	int32_t uid;
	std::string filename;
	DbDateTime start_time;
};

struct DB_NextAuctionFileToParse {
	struct Input {};

	typedef LastParsedFileName Output;
};

struct DB_ActiveAuctions {
	typedef LastParsedFileName Input;

	struct Output {
		int32_t uid;
		DbDateTime previous_time;
		DbDateTime time;
		DbDateTime estimated_end_min;
		DbDateTime estimated_end_max;
		uint16_t category;
		uint8_t duration_type;
		int64_t bid_price;
		int64_t price;
		std::string seller;
		int8_t bid_flag;
	};
};

class P0LoadActiveAuctions : public Object {
	DECLARE_CLASSNAME(P0LoadActiveAuctions, 0)
public:
	P0LoadActiveAuctions();

	typedef std::function<void(bool success, int result, const AUCTION_FILE* auctionData, std::string lastParsedFile)>
	    LoadCallback;

	void load(LoadCallback callback);

protected:
	void loadLastFileName();
	void doneLoadingLastFilename(DbQueryJob<DB_NextAuctionFileToParse>* queryJob, int status);
	void doneLoadingAuctions(DbQueryJob<DB_ActiveAuctions>* queryJob, int status);
	bool importAuctions(const std::string& lastParsedFile,
	                    DbQueryJob<DB_ActiveAuctions>* queryJob,
	                    AUCTION_FILE* auctionData);

private:
	LoadCallback callback;
	DbQueryJobRef dbQuery;
	std::string lastAuctionFilename;
};
