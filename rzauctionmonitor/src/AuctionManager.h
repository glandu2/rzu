#ifndef AUCTIONMANAGER_H
#define AUCTIONMANAGER_H

#include "Core/Object.h"
#include "AuctionWorker.h"
#include <memory>
#include <unordered_map>
#include <deque>
#include <time.h>
#include <array>
#include <unordered_set>

struct TS_SC_CHARACTER_LIST;
struct TS_SC_LOGIN_RESULT;
struct TS_SC_AUCTION_SEARCH;

class AuctionManager : public Object {
	DECLARE_CLASS(AuctionManager)

public:
	AuctionManager();

	void start();
	void stop();
	void reloadAccounts();

	std::unique_ptr<AuctionWorker::AuctionRequest> getNextRequest();
	void addAuctionInfo(const AuctionWorker::AuctionRequest* request, uint32_t uid, const char* data, int len);
	void onAuctionSearchCompleted(bool success, int pageTotal, std::unique_ptr<AuctionWorker::AuctionRequest> auctionRequest);

private:
	enum DiffType {
		D_Added = 0,
		D_Updated = 1,
		D_Deleted = 2,
		D_Unmodified = 4,
		D_Unrecognized = 1000
	};

	#pragma pack(push, 1)
	struct Header {
		int32_t size;
		int16_t version;
		int16_t flag;
		int64_t time;
		int16_t category;
	};
	#pragma pack(pop)

	struct AuctionInfo {
		enum Flag {
			AIF_NotProcessed,
			AIF_Added,
			AIF_Updated,
			AIF_Unmodifed
		};

		const uint32_t uid;
		Flag flag;
		uint64_t time;
		int32_t category;
		int32_t page;
		std::vector<uint8_t> data;

		AuctionInfo(uint32_t uid, Flag flag, int32_t category, int32_t page, const char* data, int len)
			: uid(uid), flag(flag), category(category), page(page)
		{
			this->time = ::time(nullptr);
			if(data && len > 0)
				 this->data = std::vector<uint8_t>(data, data + len);
		}
	};

private:
	void onClientStopped(IListener* instance);
	void addRequest(int category, int page);

	bool isAllRequestProcessed();
	void onAllRequestProcessed();
	void processRemovedAuctions(std::unordered_map<uint32_t, AuctionInfo>& auctionInfos);
	static void postProcessAuctionInfos(std::unordered_map<uint32_t, AuctionInfo>& auctionInfos);

	void dumpAuctions();
	template<class Container> static void serializeAuctionInfos(const Container& auctionInfos, bool includeUnmodified, std::vector<uint8_t>& output);
	void writeAuctionDataToFile(const std::vector<uint8_t>& data, const char* suffix);
	static int getAuctionDiffType(AuctionInfo::Flag flag);

	template<typename T> static const AuctionInfo& getAuctionInfoFromValue(const std::pair<T, AuctionInfo>& val) { return val.second; }
	static const AuctionInfo& getAuctionInfoFromValue(const AuctionInfo& val) { return val; }

private:
	std::vector<std::unique_ptr<AuctionWorker>> clients;
	std::deque<std::unique_ptr<AuctionWorker>> stoppingClients;
	std::deque<std::unique_ptr<AuctionWorker::AuctionRequest>> pendingRequests;

	std::unordered_map<uint32_t, AuctionInfo> auctionsState;
	std::vector<AuctionInfo> auctionActives;
	std::vector<uint8_t> fileData; //cache allocated memory

	int totalPages;
	static const size_t CATEGORY_MAX_INDEX = 18;
	size_t currentCategory;
};


#endif // AUCTIONMANAGER_H
