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

class AuctionWorker : public Object, public IListener {
	DECLARE_CLASS(AuctionWorker)
public:
	struct AuctionRequest {
		int category;
		int page;
		int failureNumber;

		AuctionRequest(int category, int page) : category(category), page(page), failureNumber(0) {}
	};
	typedef void (*StoppedCallback)(IListener* instance, AuctionWorker* worker);
public:
	AuctionWorker(AuctionManager* auctionManager,
				  cval<std::string>& ip,
				  cval<int>& port,
				  cval<int>& serverIdx,
				  cval<int>& delayTime,
				  cval<bool>& useRsa,
				  cval<int>& autoRecoDelay,
				  const std::string& account,
				  const std::string& password,
				  const std::string& playername);

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

	static void onClosed(IListener* instance);

private:
	AuctionManager* auctionManager;
	GameSession gameSession;
	AuthSession authSession;

	bool isConnected;
	bool idle;
	Timer<AuctionWorker> auctionDelayTimer, auctionSearchTimer, reconnectTimer;
	std::unique_ptr<AuctionRequest> request;

	Callback<StoppedCallback> stopCallback;
};


#endif // AUCTIONMANAGER_H
