#ifndef AGGREGATOR_H
#define AGGREGATOR_H

#include <stdint.h>
#include <unordered_map>
#include "Core/Object.h"
#include "HttpClientSession.h"
#include "IParser.h"

struct AUCTION_FILE;
struct AUCTION_INFO;
struct TS_ITEM_BASE_INFO;
class AuctionComplexDiffWriter;

class Aggregator : public IParser, public Object {
	DECLARE_CLASSNAME(Aggregator, 0)
public:
	Aggregator();

	bool isFull() { return httpClientSession.getPendingNumber() > 100; }

	bool parseAuctions(AuctionComplexDiffWriter* auctionWriter);

	bool sendToWebServer(const std::string& data);

	void exportState(std::string filename, const std::string& lastParsedFile);
	void importState(std::string filename, std::string& lastParsedFile);

protected:
	void updateCurrentDateAndCompute(time_t date);
	int compareWithCurrentDate(time_t other);
	void computeStatisticsOfDay();
	bool skipItem(const AUCTION_INFO& auctionInfo, const TS_ITEM_BASE_INFO& item);

private:
	struct AuctionSummary {
		int32_t code;
		int16_t enhance;
		int64_t price;
		bool isSold;
		int64_t count;

		AuctionSummary() : price(0), isSold(false), count(0) {}
	};
	struct DayAggregation {
		struct PriceInfo {
			int64_t itemNumber;
			int64_t minPrice;
			int64_t maxPrice;
			int64_t avgPrice;

			PriceInfo() : itemNumber(0), minPrice(-1), maxPrice(-1), avgPrice(-1) {}
			void updateAggregatedStats(const AuctionSummary& summary);
		};

		PriceInfo allItems;
		PriceInfo soldItems;
	};
	enum Category {
		C_Weapon = 0,
		C_Armor = 1,
		C_Shield = 2,
		C_Helmet = 3,
		C_Gloves = 4,
		C_Boots = 5,
		C_Belt = 6,
		C_Cloak = 7,
		C_Accessory = 8,
		C_DecorativeItem = 9,
		C_SkillCard = 10,
		C_SummonerCard = 11,
		C_Bag = 12,
		C_StrikeCube = 13,
		C_DefenseCube = 14,
		C_SkillCube = 15,
		C_SoulStone = 16,
		C_CraftMaterials = 17,
		C_Supplies = 18
	};


	HttpClientSession httpClientSession;
	cval<std::string>& url;
	cval<std::string>& pwd;
	time_t currentDate;

	std::unordered_map<uint32_t, AuctionSummary> auctionsByUid;
};

#endif
