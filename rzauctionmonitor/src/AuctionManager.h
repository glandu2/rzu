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

	void addRequest(int category, int page);

	void addAuctionInfo(const AuctionWorker::AuctionRequest* request, uint32_t uid, const char* data, int len);
	void onAuctionSearchCompleted(bool success, int pageTotal, std::unique_ptr<AuctionWorker::AuctionRequest> auctionRequest);
	std::unique_ptr<AuctionWorker::AuctionRequest> getNextRequest();
	bool isAllRequestProcessed();

	void onAllRequestProcessed();
	bool dumpAuctions();

protected:
	void onClientStopped(IListener* instance);

private:
	std::vector<std::unique_ptr<AuctionWorker>> clients;
	std::deque<std::unique_ptr<AuctionWorker>> stoppingClients;
	std::deque<std::unique_ptr<AuctionWorker::AuctionRequest>> pendingRequests;

	struct AuctionInfo {
		uint32_t uid;
		uint64_t time;
		int32_t category;
		int32_t page;
		std::vector<uint8_t> data;

		AuctionInfo(uint32_t uid, int32_t category, int32_t page, const char* data, int len)
			: uid(uid), category(category), page(page), data(data, data + len)
		{
			this->time = ::time(NULL);
		}
		AuctionInfo() : uid(0), category(0), page(0) {}
	};

	std::deque<AuctionInfo> auctionInfos;

	int totalPages;
	int currentCategory;
};


#endif // AUCTIONMANAGER_H
