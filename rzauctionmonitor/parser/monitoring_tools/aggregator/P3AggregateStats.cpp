#include "P3AggregateStats.h"
#include "GameClient/TS_SC_AUCTION_SEARCH.h"

P3AggregateStats::P3AggregateStats()
    : PipelineStep<std::unique_ptr<AuctionDumpToAggregate>,
                   std::tuple<std::string, time_t, AUCTION_FILE, std::unordered_map<uint32_t, AuctionSummary>>,
                   char>(2),
      work(this, &P3AggregateStats::processWork, &P3AggregateStats::afterWork) {}

void P3AggregateStats::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	work.run(item);
}

int P3AggregateStats::processWork(std::shared_ptr<WorkItem> workItem) {
	AuctionDumpToAggregate* auctionFileToAggregate = workItem->getSource().get();

	if(auctionFileToAggregate->isNewDay) {
		if(!auctionsByUid.empty()) {
			log(LL_Info,
			    "Computing statistics of %d auctions for day %d\n",
			    (int) auctionsByUid.size(),
			    (int) auctionFileToAggregate->previousTimestamp);
			addResult(workItem,
			          std::make_tuple(auctionFileToAggregate->filename,
			                          auctionFileToAggregate->previousTimestamp,
			                          auctionFileToAggregate->auctionFile,
			                          std::move(auctionsByUid)),
			          0);
			auctionsByUid.clear();
		} else {
			log(LL_Info,
			    "Not computing statistics for day %d, no auction\n",
			    (int) auctionFileToAggregate->previousTimestamp);
		}
	} else if(auctionsByUid.empty()) {
		log(LL_Warning, "Not a new day but auctionsByUid is empty\n");
	}

	log(LL_Debug,
	    "Aggregating stats for %d auctions for date %d\n",
	    (int) auctionFileToAggregate->auctionFile.auctions.size(),
	    (int) auctionFileToAggregate->timestamp);

	for(size_t i = 0; i < auctionFileToAggregate->auctionFile.auctions.size(); i++) {
		const AUCTION_INFO& auctionInfo = auctionFileToAggregate->auctionFile.auctions[i];
		if(!auctionInfo.data.data())
			continue;

		TS_SEARCHED_AUCTION_INFO item;
		MessageBuffer structBuffer(auctionInfo.data.data(), auctionInfo.data.size(), EPIC_9_5_3);

		item.deserialize(&structBuffer);
		if(!structBuffer.checkFinalSize()) {
			log(LL_Error, "Invalid item data content for uid %u, can't deserialize\n", auctionInfo.uid);
			return false;
		} else if(structBuffer.getParsedSize() != auctionInfo.data.size()) {
			log(LL_Error, "Invalid item data size for uid %u, can't deserialize safely\n", auctionInfo.uid);
			return false;
		}

		if(skipItem(auctionInfo, item.auction_details.item_info))
			continue;

		AuctionSummary& summary = auctionsByUid[auctionInfo.uid];
		summary.code = item.auction_details.item_info.code;
		summary.enhance = item.auction_details.item_info.enhance;

		if(!summary.isSold) {
			if(item.auction_details.item_info.count > 0)
				summary.count = item.auction_details.item_info.count;
			else
				summary.count = 1;

			summary.price = auctionInfo.bid_price / summary.count;
		}

		if(auctionInfo.diffType == D_Deleted) {
			if(auctionInfo.bid_flag != BF_NoBid) {
				summary.isSold = true;
				summary.price = auctionInfo.bid_price / summary.count;
			}

			if(auctionInfo.price != 0 && auctionInfo.estimatedEndTimeMin != 0 &&
			   (auctionInfo.time + 120) < auctionInfo.estimatedEndTimeMin) {
				summary.isSold = true;
				summary.price = auctionInfo.price / summary.count;
			}
		}
	}

	return 0;
}

bool P3AggregateStats::skipItem(const AUCTION_INFO& auctionInfo, const TS_ITEM_BASE_INFO& item) {
	switch(auctionInfo.category) {
		case C_Weapon:
		case C_Armor:
		case C_Shield:
		case C_Helmet:
		case C_Gloves:
		case C_Boots:
		case C_Cloak:
		case C_Accessory:
			return item.enhance > 0 || item.awaken_option.value[0] != 0 || item.awaken_option.value[1] != 0 ||
			       item.awaken_option.value[2] != 0 || item.awaken_option.value[3] != 0 ||
			       item.awaken_option.value[4] != 0 || item.random_type[0] != 0 || item.random_type[1] != 0 ||
			       item.random_type[2] != 0 || item.random_type[3] != 0 || item.random_type[4] != 0 ||
			       item.random_type[5] != 0 || item.random_type[6] != 0 || item.random_type[7] != 0 ||
			       item.random_type[8] != 0 || item.random_type[9] != 0;
		case C_Belt:
			return item.enhance > 0 || item.socket[0] > 0;
		case C_SkillCard:
			return item.enhance > 1;
		case C_SummonerCard:
			return (item.flag & 0x80000000) != 0 || item.enhance > 0;  // exclude tamed and staged

		default:
			return false;
	}
}

void P3AggregateStats::afterWork(std::shared_ptr<WorkItem> item, int status) {
	workDone(item, status);
}
