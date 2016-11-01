#ifndef AUCTIONGENERICWRITER_H
#define AUCTIONGENERICWRITER_H

#include <unordered_map>
#include <vector>
#include <stdint.h>
#include "AuctionUid.h"
#include "IAuctionData.h"
#include <memory>

#include "AuctionCommonWriter.h"

template<class AuctionData, class AUCTION_FILE_TYPE>
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

	template<typename ...Args>
	void dumpAuctions(const std::string& auctionDir, const std::string& auctionFile, bool dumpDiff, bool dumpFull, Args... args)
	{
		time_t dumpTimeStamp = categoryTimeManager.getLastEndCategoryTime();
		if(dumpTimeStamp == 0) {
			log(LL_Warning, "Last category end timestamp is 0, using current timestamp\n");
			dumpTimeStamp = ::time(nullptr);
		}

		if(dumpDiff) {
			dumpAuctions(fileData, false, args...);
			writeAuctionDataToFile(auctionDir, auctionFile, fileData, dumpTimeStamp, "_diff");
		}

		if(dumpFull) {
			dumpAuctions(fileData, true, args...);
			writeAuctionDataToFile(auctionDir, auctionFile, fileData, dumpTimeStamp, "_full");
		}
	}

	template<typename ...Args>
	void dumpAuctions(std::vector<uint8_t> &output, bool doFullDump, Args... args)
	{
		AUCTION_FILE_TYPE file;

		exportDump(doFullDump, file, args...);
		serialize(file, output);
	}

	template<typename ...Args>
	void exportDump(bool doFullDump, AUCTION_FILE_TYPE& auctionFile, Args... args)
	{
		categoryTimeManager.serializeHeader(auctionFile.header, doFullDump ? DT_Full : DT_Diff);

		auctionFile.auctions.reserve(auctions.size());

		auto it = auctions.begin();
		for(; it != auctions.end(); ++it) {
			AuctionData* auctionInfo = it->second.get();
			if(!doFullDump && !auctionInfo->outputInPartialDump())
				continue;

			typename decltype(auctionFile.auctions)::value_type auctionItem;
			auctionInfo->serialize(&auctionItem, args...);

			auctionFile.auctions.push_back(auctionItem);
		}
	}

	void importDump(AUCTION_FILE_TYPE *auctionFile)
	{
		auctions.clear();
		for(size_t i = 0; i < auctionFile->auctions.size(); i++) {
			auto& auctionInfo = auctionFile->auctions[i];
			auctions.insert(std::make_pair(auctionInfo.uid, std::move(std::unique_ptr<AuctionData>(AuctionData::createFromDump(&auctionInfo)))));
		}
	}

	void serialize(const AUCTION_FILE_TYPE& auctionFile, std::vector<uint8_t> &output)
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
	std::vector<uint8_t> fileData; //cache allocated memory
};

#endif // AUCTIONSIMPLEDIFF_H
