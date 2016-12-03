#ifndef AGGREGATOR_H
#define AGGREGATOR_H

#include <stdint.h>
#include <unordered_map>
#include "Core/Object.h"
#include "HttpClientSession.h"

struct AUCTION_FILE;

class Aggregator : public Object {
	DECLARE_CLASSNAME(Aggregator, 0)
public:
	Aggregator();

	bool isFull() { return httpClientSession.getPendingNumber() > 10; }

	bool parseFile(const char* filename);
	time_t parseAuctions(const AUCTION_FILE& auctionFile);

	bool writeToFile(const char* filename, const std::string& data);
	bool sendToWebServer(const std::string& data);

	void exportState(std::string filename, const std::string& lastParsedFile);
	void importState(std::string filename, std::string& lastParsedFile);

protected:
	void updateCurrentDateAndCompute(time_t date);
	int compareWithCurrentDate(time_t other);
	void computeStatisticsOfDay();

private:
	struct AuctionSummary {
		int64_t price;
		bool isSold;
	};

	HttpClientSession httpClientSession;
	time_t currentDate;
	std::unordered_map<uint32_t, std::vector<AuctionSummary>> auctionData;
	cval<std::string>& url;
};

#endif
