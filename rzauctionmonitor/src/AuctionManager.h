#ifndef AUCTIONMANAGER_H
#define AUCTIONMANAGER_H

#include "Core/Object.h"
#include "AuctionWorker.h"
#include "AuctionWriter.h"
#include <memory>
#include <deque>
#include <time.h>

struct TS_SC_CHARACTER_LIST;
struct TS_SC_LOGIN_RESULT;
struct TS_SC_AUCTION_SEARCH;
class IWritableConsole;

class AuctionManager : public Object, public IListener {
	DECLARE_CLASS(AuctionManager)

public:
	AuctionManager();

	void start();
	void stop();
	void reloadAccounts();
	void loadAccounts();

	std::unique_ptr<AuctionWorker::AuctionRequest> getNextRequest();
	void addAuctionInfo(const AuctionWorker::AuctionRequest* request, uint32_t uid, const uint8_t *data, int len);
	void onAuctionSearchCompleted(bool success, int pageTotal, std::unique_ptr<AuctionWorker::AuctionRequest> auctionRequest);

	static void onReloadAccounts(IWritableConsole* console, const std::vector<std::string>& args);

private:
	static void onClientStoppedStatic(IListener* instance, AuctionWorker* worker);
	void onClientStopped(AuctionWorker* worker);
	void onAccountReloadTimer();

	void addRequest(int category, int page);

	bool isAllRequestProcessed();
	void onAllRequestProcessed();

private:
	AuctionWriter auctionWriter;
	std::vector<std::unique_ptr<AuctionWorker>> clients;
	std::vector<std::unique_ptr<AuctionWorker>> stoppingClients;

	std::deque<std::unique_ptr<AuctionWorker::AuctionRequest>> pendingRequests;

	int totalPages;
	static const size_t CATEGORY_MAX_INDEX = 18;
	size_t currentCategory;
	bool firstDump;

	bool reloadingAccounts;
	Timer<AuctionManager> accountReloadTimer;
	static AuctionManager* instance;
};


#endif // AUCTIONMANAGER_H
