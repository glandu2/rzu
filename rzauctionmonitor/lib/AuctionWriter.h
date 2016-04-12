#ifndef AUCTIONWRITER_H
#define AUCTIONWRITER_H

#include "Core/Object.h"
#include <unordered_map>
#include <vector>
#include <array>
#include "Extern.h"
#include <stdint.h>
#include "AuctionFile.h"

class RZAUCTION_EXTERN AuctionWriter : public Object {
	DECLARE_CLASSNAME(AuctionWriter, 0)

public:
	AuctionWriter(size_t categoryCount);

	void setDiffInputMode(bool diffMode);
	void addAuctionInfo(uint32_t uid, uint64_t time, uint16_t category, const uint8_t *data, int len);
	void addAuctionInfoDiff(uint32_t uid, uint64_t time, uint64_t previousTime, DiffType diffType, uint16_t category, const uint8_t *data, int len);

	void beginCategory(size_t category, time_t time);
	void endCategory(size_t category, time_t time);
	void dumpAuctions(const std::string &auctionDir, const std::string &auctionFile, bool dumpDiff, bool dumpFull);

	bool hasAuction(uint32_t uid);
	size_t getAuctionCount() { return auctionsState.size(); }

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

	struct AuctionInfo {
		enum Flag {
			AIF_NotProcessed,
			AIF_Unmodifed,
			AIF_Base,
			AIF_Added,
			AIF_Updated,
			AIF_Deleted
		};

		uint32_t uid;
		Flag flag;
		int32_t category;
		std::vector<uint8_t> data;

		AuctionInfo(uint32_t uid, Flag flag, uint64_t time, uint64_t previousTime, int32_t category, const uint8_t* data, int len)
			: uid(uid), flag(flag), category(category), time(time), previousTime(previousTime)
		{
			if(data && len > 0)
				 this->data = std::vector<uint8_t>(data, data + len);
		}

		AuctionInfo& operator=(const AuctionInfo& other) {
			this->uid = other.uid;
			this->flag = other.flag;
			this->category = other.category;
			this->data = other.data;
			this->time = other.time;
			this->previousTime = other.previousTime;
			return *this;
		}

		void updateTime(uint64_t newTime) {
			previousTime = time;
			time = newTime;
		}

		void updateTimes(uint64_t newTime, uint64_t previousTime) {
			this->previousTime = previousTime;
			this->time = newTime;
		}

		uint64_t getTime() const { return time; }
		uint64_t getPreviousTime() const { return previousTime; }

	private:
		uint64_t time;
		uint64_t previousTime;
	};

private:
	void processRemovedAuctions(std::unordered_map<uint32_t, AuctionInfo>& auctionInfos);
	void postProcessAuctionInfos(std::unordered_map<uint32_t, AuctionInfo>& auctionInfos);

	template<class Container> void serializeAuctionInfos(const Container& auctionInfos, bool doFullDump, std::vector<uint8_t>& output);
	void writeAuctionDataToFile(std::string auctionsDir, std::string auctionsFile, const std::vector<uint8_t>& data, time_t fileTimeStamp, const char* suffix);
	static int getAuctionDiffType(AuctionInfo::Flag flag);

	template<typename T> static const AuctionInfo& getAuctionInfoFromValue(const std::pair<T, AuctionInfo>& val) { return val.second; }
	static const AuctionInfo& getAuctionInfoFromValue(const AuctionInfo& val) { return val; }

	void adjustCategoryTimeRange(size_t category, time_t time);
	CategoryTime& getCategoryTime(size_t category);
	time_t getEstimatedPreviousCategoryBeginTime(size_t category);
	time_t getLastEndCategoryTime();

private:
	bool firstAuctions;
	bool diffMode;
	std::unordered_map<uint32_t, AuctionInfo> auctionsState;
	std::vector<uint8_t> fileData; //cache allocated memory
	int fileNumber;

	std::vector<CategoryTime> categoryTime;
};

#endif // AUCTIONWRITER_H
