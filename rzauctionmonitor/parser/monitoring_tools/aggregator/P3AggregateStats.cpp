#include "P3AggregateStats.h"
#include "Core/Utils.h"
#include "GameClient/TS_SC_AUCTION_SEARCH.h"
#include "errno.h"

P3AggregateStats::P3AggregateStats()
    : PipelineStep<std::pair<PipelineState, std::vector<AUCTION_FILE>>,
                   std::pair<PipelineAggregatedState, std::unordered_map<uint32_t, AuctionSummary>>>(0, 1, 1),
      work(this, &P3AggregateStats::processWork, &P3AggregateStats::afterWork) {}

void P3AggregateStats::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	work.run(item);
}

int P3AggregateStats::aggregateStats(const std::vector<AUCTION_FILE>& auctions,
                                     std::unordered_map<uint32_t, AuctionSummary>& auctionsByUid) {
	for(const AUCTION_FILE& auctionFile : auctions) {
		for(const AUCTION_INFO& auctionInfo : auctionFile.auctions) {
			if(!auctionInfo.data.data())
				continue;

			TS_SEARCHED_AUCTION_INFO item;
			MessageBuffer structBuffer(auctionInfo.data.data(), auctionInfo.data.size(), auctionInfo.epic);

			item.deserialize(&structBuffer);
			if(!structBuffer.checkFinalSize()) {
				log(LL_Error, "Invalid item data content for uid %u, can't deserialize\n", auctionInfo.uid);
				return -ENOENT;
			} else if(structBuffer.getParsedSize() != auctionInfo.data.size()) {
				log(LL_Error,
				    "Invalid item data size for uid %u, can't deserialize safely: expected size: %d, got %d, epic: "
				    "0x%X\n",
				    auctionInfo.uid,
				    structBuffer.getParsedSize(),
				    (int) auctionInfo.data.size(),
				    auctionInfo.epic);
				return -EBADF;
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
	}

	return 0;
}

int P3AggregateStats::processWork(std::shared_ptr<WorkItem> workItem) {
	auto sources = std::move(workItem->getSources());
	for(std::pair<PipelineState, std::vector<AUCTION_FILE>>& input : sources) {
		std::unordered_map<uint32_t, AuctionSummary> auctionsByUid;
		std::string timestampStr;
		struct tm timestampTm;

		Utils::getGmTime(input.first.timestamp, &timestampTm);
		Utils::stringFormat(
		    timestampStr, "%02d/%02d/%04d", timestampTm.tm_mday, timestampTm.tm_mon, timestampTm.tm_year);

		workItem->setName(timestampStr);

		log(LL_Info,
		    "Aggregating statistics of %d auctions for day %s\n",
		    (int) input.second.size(),
		    timestampStr.c_str());

		if(input.second.empty()) {
			log(LL_Error, "No auction file to aggregate for day %s, skipping\n", timestampStr.c_str());
			continue;
		} else if(input.second[0].header.dumpType != DT_Full) {
			log(LL_Error,
			    "First dump is not a full dump for day %s, this is required to store last valid state\n",
			    timestampStr.c_str());
			continue;
		}

		// Value estimated from real data (max at 2815)
		auctionsByUid.reserve(3000);
		if(aggregateStats(input.second, auctionsByUid) != 0)
			continue;

		if(!input.second.empty()) {
			PipelineAggregatedState aggregatedState;
			aggregatedState.base = std::move(input.first);
			aggregatedState.dumps = std::move(input.second);

			addResult(workItem, std::make_pair(std::move(aggregatedState), std::move(auctionsByUid)));
		} else {
			log(LL_Warning, "No auction for day %s\n", timestampStr.c_str());
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
