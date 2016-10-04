#ifndef AUCTIONWRITER_H
#define AUCTIONWRITER_H

#include "Core/Object.h"
#include <unordered_map>
#include <vector>
#include <array>
#include "Extern.h"
#include <stdint.h>
#include "AuctionFile.h"
#include "AuctionData.h"
#include "AuctionCommonWriter.h"

class RZAUCTION_EXTERN AuctionWriter : public AuctionCommonWriter {
	DECLARE_CLASSNAME(AuctionWriter, 0)

public:
	AuctionWriter(size_t categoryCount);

	void setDiffInputMode(bool diffMode);
	void addAuctionInfo(uint32_t uid, uint64_t time, uint16_t category, const uint8_t *data, size_t len);
	void addMaybeDeletedAuctionInfo(uint32_t uid, uint64_t time, uint64_t previousTime, uint32_t deletedCount, uint16_t category, const uint8_t* data, size_t len);
	void addAuctionInfoDiff(uint32_t uid, uint64_t time, uint64_t previousTime, DiffType diffType, uint16_t category, const uint8_t *data, size_t len);

	bool hasAuction(uint32_t uid);
	size_t getAuctionCount() { return auctionsState.size(); }

	void importDump(AUCTION_FILE* auctionFile);

private:
	struct CategoryTime {
		time_t previousBegin;
		time_t begin;
		time_t end;

		CategoryTime() : previousBegin(0), begin(0), end(0) {}
		void resetTimes() {
			previousBegin = begin;
			begin = end = 0;
		}
	};

private:
	virtual void resetAuctionProcess();
	virtual void serializeAuctionInfos(bool doFullDump, std::vector<uint8_t>& output);
	virtual void processRemainingAuctions();

	template<typename T> static const AuctionInfo& getAuctionInfoFromValue(const std::pair<T, AuctionInfo>& val) { return val.second; }
	static const AuctionInfo& getAuctionInfoFromValue(const AuctionInfo& val) { return val; }

private:
	bool diffMode;

	std::unordered_map<uint32_t, AuctionInfo> auctionsState;
};

#endif // AUCTIONWRITER_H
