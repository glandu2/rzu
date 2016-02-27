#ifndef AUCTIONWORKER_H
#define AUCTIONWORKER_H

#include "Core/Object.h"
#include "GameSession.h"
#include "AuthSession.h"
#include "Core/Timer.h"
#include "Core/IDelegate.h"
#include <memory>

class AuctionManager;
struct TS_SC_AUCTION_SEARCH;

class AuctionWorker : public Object {
	DECLARE_CLASS(AuctionWorker)
public:
	struct AuctionRequest {
		int category;
		int page;
		int failureNumber;

		AuctionRequest(int category, int page) : category(category), page(page), failureNumber(0) {}
	};
	typedef void (*StoppedCallback)(IListener* instance);
public:
	AuctionWorker(AuctionManager* auctionManager,
				  const std::string& playername,
				  const std::string& ip,
				  uint16_t port,
				  const std::string& account,
				  const std::string& password,
				  int serverIdx,
				  int recoDelay,
				  int autoRecoDelay,
				  bool useRsa);

	void start();
	void stop(Callback<StoppedCallback> callback);
	void onConnected();
	void onDisconnected();

	bool hasRequest() { return request.get() != nullptr; }
	void wakeUp();
	void onAuctionSearchResult(const TS_SC_AUCTION_SEARCH* packet);
	void onAuctionSearchFailed(int resultCode);

protected:
	bool doAuctionSearch();
	void onAuctionSearchTimer();

	void onAuctionTimer();
	void onReconnectTimer();

private:
	AuctionManager* auctionManager;
	GameSession gameSession;
	AuthSession authSession;

	bool isConnected;
	Timer<AuctionWorker> auctionDelayTimer, auctionSearchTimer, reconnectTimer;
	bool idle;
	std::unique_ptr<AuctionRequest> request;
};


#endif // AUCTIONMANAGER_H
