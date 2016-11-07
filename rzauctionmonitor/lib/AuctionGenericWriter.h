#ifndef AUCTIONGENERICWRITER_H
#define AUCTIONGENERICWRITER_H

#include <unordered_map>
#include <vector>
#include <stdint.h>
#include "AuctionUid.h"
#include <memory>

#include "AuctionCommonWriter.h"

template<class AuctionData>
class AuctionGenericWriter : public AuctionCommonWriter
{
public:
	enum MergeResult {
		MR_Added,
		MR_Updated
	};

public:
	AuctionGenericWriter(size_t categoryCount) : AuctionCommonWriter(categoryCount) {}
	~AuctionGenericWriter() {}

	bool hasAuction(AuctionUid uid) {
		return auctions.count(uid.get()) > 0;
	}

	size_t getAuctionCount() {
		return auctions.size();
	}

	AuctionData* getAuction(AuctionUid uid) {
		auto it = auctions.find(uid.get());
		if(it != auctions.end())
			return it->second.get();
		else
			return nullptr;
	}

	bool addAuction(AuctionData* auctionData) {
		auto insertIt = auctions.insert(std::make_pair(auctionData->getUid().get(), std::move(std::unique_ptr<AuctionData>(auctionData))));
		if(insertIt.second == false) {
			log(LL_Error, "Coulnd't insert added auction: 0x%08X\n", auctionData->getUid().get());
		}
		return insertIt.second;
	}

	void clearAuctions() {
		auctions.clear();
	}

	template<class Serializable>
	void serialize(const Serializable& auctionFile, std::vector<uint8_t> &output)
	{
		output.clear();

		MessageBuffer buffer(auctionFile.getSize(auctionFile.header.file_version), auctionFile.header.file_version);
		auctionFile.serialize(&buffer);
		if(buffer.checkFinalSize() == false) {
			log(LL_Error, "Wrong buffer size, size: %d, field: %s\n", buffer.getSize(), buffer.getFieldInOverflow().c_str());
		} else {
			output.insert(output.end(), buffer.getData(), buffer.getData() + buffer.getSize());
		}
	}

protected:
	template<typename OnEach>
	void beginProcess(OnEach callback) {
		purgeAuctions();
		forEachAuction(callback);
	}

	template<typename OnEach>
	void endProcess(OnEach callback) {
		forEachAuction(callback);
		categoryTimeManager.resetCategoryTime();
	}

	template<typename OnEachCallback>
	void forEachAuction(OnEachCallback callback) {
		auto it = auctions.begin();
		for(; it != auctions.end(); ++it) {
			AuctionData* auction = it->second.get();
			callback(auction);
		}
	}

	void purgeAuctions() {
		auto it = auctions.begin();
		for(; it != auctions.end();) {
			AuctionData* auction = it->second.get();
			if(auction->isInFinalState())
				it = auctions.erase(it);
			else
				++it;
		}
	}

private:
	AuctionGenericWriter(const AuctionGenericWriter& other) = delete;
	void operator=(const AuctionGenericWriter& other) = delete;

	std::unordered_map<uint32_t, const std::unique_ptr<AuctionData>> auctions;
};

#endif // AUCTIONSIMPLEDIFF_H
