#ifndef AUCTIONGENERICWRITER_H
#define AUCTIONGENERICWRITER_H

#include "AuctionUid.h"
#include "Packet/MessageBuffer.h"
#include <algorithm>
#include <deque>
#include <memory>
#include <stdint.h>
#include <unordered_map>
#include <vector>

#include "AuctionCommonWriter.h"
#include "flat_map/flat_map.hpp"

template<class AuctionData> class AuctionGenericWriter : public AuctionCommonWriter {
public:
	enum MergeResult { MR_Added, MR_Updated };

public:
	AuctionGenericWriter(size_t categoryCount) : AuctionCommonWriter(categoryCount) {
		// auctions.container.reserve(20000);
	}
	~AuctionGenericWriter() {}

	bool hasAuction(AuctionUid uid) { return auctions.count(uid.get()) > 0; }

	size_t getAuctionCount() { return auctions.size(); }

	template<class UnaryPredicate> size_t getAuctionCountIf(UnaryPredicate p) {
		return std::count_if(auctions.begin(),
		                     auctions.end(),
		                     [&p](std::pair<const uint32_t, const std::unique_ptr<AuctionData>>& data) -> bool {
			                     return p(data.second.get());
		                     });
	}

	AuctionData* getAuction(AuctionUid uid) {
		auto it = auctions.find(uid.get());
		if(it != auctions.end())
			return it->second.get();
		else
			return nullptr;
	}

	bool addAuction(AuctionData* auctionData) {
		auto insertIt = auctions.insert(
		    std::make_pair(auctionData->getUid().get(), std::move(std::unique_ptr<AuctionData>(auctionData))));
		if(insertIt.second == false) {
			log(LL_Error, "Coulnd't insert added auction: 0x%08X\n", auctionData->getUid().get());
		}
		return insertIt.second;
	}

	void clearAuctions() { auctions.clear(); }

	template<class Serializable> void serialize(const Serializable& auctionFile, std::vector<uint8_t>& output) {
		output.clear();

		MessageBuffer buffer(auctionFile.getSize(auctionFile.header.file_version), auctionFile.header.file_version);
		auctionFile.serialize(&buffer);
		if(buffer.checkFinalSize() == false) {
			log(LL_Error,
			    "Wrong buffer size, size: %d, field: %s\n",
			    buffer.getSize(),
			    buffer.getFieldInOverflow().c_str());
		} else {
			output.insert(output.end(), buffer.getData(), buffer.getData() + buffer.getSize());
		}
	}

protected:
	template<typename OnEach> void beginProcess(OnEach callback) {
		// Purge auctions in final state and call callback on others
		auto it = auctions.begin();
		auto itEnd = auctions.end();
		for(; it != itEnd;) {
			AuctionData* auction = it->second.get();
			if(auction->isInFinalState()) {
				it = auctions.erase(it);
				itEnd = auctions.end();
			} else {
				callback(auction);
				++it;
			}
		}
	}

	template<typename OnEach> void endProcess(OnEach callback) {
		forEachAuction(callback);
		categoryTimeManager.resetCategoryTime();
	}

	template<typename OnEachCallback> void forEachAuction(OnEachCallback callback) {
		auto it = auctions.begin();
		auto itEnd = auctions.end();
		for(; it != itEnd; ++it) {
			AuctionData* auction = it->second.get();
			callback(auction);
		}
	}

	void purgeAuctions() {
		auto it = auctions.begin();
		auto itEnd = auctions.end();
		for(; it != itEnd;) {
			AuctionData* auction = it->second.get();
			if(auction->isInFinalState()) {
				it = auctions.erase(it);
				itEnd = auctions.end();
			} else {
				++it;
			}
		}
	}

private:
	AuctionGenericWriter(const AuctionGenericWriter& other);
	void operator=(const AuctionGenericWriter& other);

	// std::unordered_map<uint32_t, const std::unique_ptr<AuctionData>> auctions;
	fc::flat_map<std::deque<std::pair<uint32_t, std::unique_ptr<AuctionData>>>> auctions;
};

#endif  // AUCTIONSIMPLEDIFF_H
