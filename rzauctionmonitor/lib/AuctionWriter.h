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

		AuctionInfo(uint32_t uid, Flag flag, uint64_t time, uint64_t previousTime, int32_t category, const uint8_t* data, int len)
			: uid(uid), flag(flag), category(category), time(time), previousTime(previousTime), estimatedEnd(0)
		{
			if(data && len > 0)
				updateData(data, len);
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

		void updateData(const uint8_t* newData, int len) {
#pragma pack(push, 1)
			struct AuctionDataEnd {
				int8_t duration_type;
				int64_t bid_price;
				int64_t price;
				char seller[31];
				int8_t flag;
			};
#pragma pack(pop)

			time_t end = 0;
			AuctionDataEnd* dataOld = nullptr;
			std::vector<uint8_t> oldData(newData, newData + len);
			oldData.swap(data);

			if(oldData.size() > sizeof(AuctionDataEnd))
				dataOld = (AuctionDataEnd*) (oldData.data() + oldData.size() - sizeof(AuctionDataEnd));

			AuctionDataEnd* dataNew = (AuctionDataEnd*) (newData + len - sizeof(AuctionDataEnd));

			if((dataOld && dataOld->duration_type == dataNew->duration_type) || previousTime == 0)
				return;

			switch(dataNew->duration_type) {
				case 3: end = previousTime + 72*3600; break;
				case 2: end = previousTime + 24*3600; break;
				case 1: end = previousTime + 6*3600; break;
			}
			if(end && (end > estimatedEnd || estimatedEnd == 0))
				estimatedEnd = end;
		}

		uint64_t getTime() const { return time; }
		uint64_t getPreviousTime() const { return previousTime; }
		const std::vector<uint8_t>& getData() const { return data; }
		uint64_t getEstimatedEnd() const { return estimatedEnd; }

	private:
		uint64_t time;
		uint64_t previousTime;
		std::vector<uint8_t> data;
		time_t estimatedEnd;
	};

private:
	void processRemovedAuctions(std::unordered_map<uint32_t, AuctionInfo>& auctionInfos);
	void postProcessAuctionInfos(std::unordered_map<uint32_t, AuctionInfo>& auctionInfos);

	template<class Container> void serializeAuctionInfos(const Container& auctionInfos, bool doFullDump, std::vector<uint8_t>& output);
	void writeAuctionDataToFile(std::string auctionsDir, std::string auctionsFile, const std::vector<uint8_t>& data, time_t fileTimeStamp, const char* suffix);
	static DiffType getAuctionDiffType(AuctionInfo::Flag flag);

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
