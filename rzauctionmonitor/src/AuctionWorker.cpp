#include "AuctionWorker.h"
#include "AuctionManager.h"
#include "GlobalConfig.h"
#include "TS_SC_AUCTION_SEARCH.h"

AuctionWorker::AuctionWorker(AuctionManager* auctionManager,
                             cval<std::string>& ip,
                             cval<int>& port,
                             cval<int>& serverIdx,
                             cval<int>& delayTime,
                             cval<bool>& useRsa,
                             cval<int>& autoRecoDelay,
                             const std::string& account,
                             const std::string& password,
                             const std::string& playername)
    : auctionManager(auctionManager),
      gameSession(this, playername, autoRecoDelay, CONFIG_GET()->client.version),
      authSession(
          &gameSession, ip, port, serverIdx, delayTime, useRsa, account, password, CONFIG_GET()->client.version),
      isConnected(false),
      idle(true) {
	log(LL_Info,
	    "Initialized worker using character %s on GS %d@%s:%d\n",
	    playername.c_str(),
	    serverIdx.get(),
	    ip.get().c_str(),
	    port.get());
}

void AuctionWorker::start() {
	if(authSession.getState() == Stream::UnconnectedState && gameSession.getState() == Stream::UnconnectedState) {
		authSession.connect();
		int reconnectTimout = CONFIG_GET()->client.recoTimeout.get() + CONFIG_GET()->client.recoDelay.get();
		reconnectTimer.start(this, &AuctionWorker::onReconnectTimer, reconnectTimout, reconnectTimout);
	}
}

void AuctionWorker::stop(Callback<AuctionWorker::StoppedCallback> callback) {
	stopCallback = callback;
	authSession.disconnect(Callback<AuthSession::DisconnectedCallback>(this, &AuctionWorker::onClosed));
}

void AuctionWorker::onClosed(IListener* instance) {
	AuctionWorker* thisInstance = (AuctionWorker*) instance;

	CALLBACK_CALL(thisInstance->stopCallback, thisInstance);
}

void AuctionWorker::onConnected() {
	isConnected = true;
	idle = true;
	reconnectTimer.stop();
	wakeUp();
}

void AuctionWorker::wakeUp() {
	if(!request && isConnected && idle) {
		onAuctionTimer();
	}
}

void AuctionWorker::onAuctionTimer() {
	idle = !doAuctionSearch();
}

bool AuctionWorker::doAuctionSearch() {
	if(!request && isConnected) {
		request = auctionManager->getNextRequest();
		if(request) {
			gameSession.auctionSearch(request->category, request->page);
			auctionSearchTimer.start(
			    this, &AuctionWorker::onAuctionSearchTimer, CONFIG_GET()->client.auctionSearchTimeout.get(), 0);
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

void AuctionWorker::onAuctionSearchTimer() {
	if(request) {
		log(LL_Error, "Auction search response timeout, reconnecting\n");
		gameSession.abortSession();
	}
}

void AuctionWorker::onAuctionSearchResult(const TS_SC_AUCTION_SEARCH* packet) {
	auctionSearchTimer.stop();
	auctionDelayTimer.start(this, &AuctionWorker::onAuctionTimer, CONFIG_GET()->client.auctionSearchDelay.get(), 0);

	if(request) {
		const int auctionInfoSize = (packet->size - sizeof(TS_SC_AUCTION_SEARCH)) / 40;
		for(int i = 0; i < packet->auction_info_count; i++) {
			const TS_AUCTION_INFO* auctionInfo = (const TS_AUCTION_INFO*) &packet->auctionInfos[auctionInfoSize * i];
			auctionManager->addAuctionInfo(
			    request.get(), auctionInfo->uid, (const uint8_t*) auctionInfo, auctionInfoSize);
		}
		log(LL_Info,
		    "Auction search of category %d, page %d/%d found %d results\n",
		    request->category,
		    request->page,
		    packet->total_page_count,
		    packet->auction_info_count);
		auctionManager->onAuctionSearchCompleted(true, packet->total_page_count, std::move(request));
	} else {
		log(LL_Error,
		    "Received auction results without request: page: %d/%d, %d items\n",
		    packet->page_num,
		    packet->total_page_count,
		    packet->auction_info_count);
	}
}

void AuctionWorker::onAuctionSearchFailed(int resultCode) {
	auctionSearchTimer.stop();
	auctionDelayTimer.start(this, &AuctionWorker::onAuctionTimer, CONFIG_GET()->client.auctionSearchDelay.get(), 0);

	if(request) {
		log(LL_Warning,
		    "Auction search of category %d, page %d failed with code %d\n",
		    request->category,
		    request->page,
		    resultCode);
		auctionManager->onAuctionSearchCompleted(false, 0, std::move(request));
	} else {
		log(LL_Error, "Received auction search error %d without request\n", resultCode);
	}
}

void AuctionWorker::onDisconnected() {
	isConnected = false;
	idle = true;
	auctionDelayTimer.stop();
	auctionSearchTimer.stop();
	if(request) {
		int page = request->page;
		log(LL_Info, "Disconnected while doing request\n");
		auctionManager->onAuctionSearchCompleted(false, page, std::move(request));
	}

	if(!authSession.isDisconnectRequested()) {
		int reconnectTimout = CONFIG_GET()->client.recoTimeout.get() + CONFIG_GET()->client.recoDelay.get();
		reconnectTimer.start(this, &AuctionWorker::onReconnectTimer, reconnectTimout, reconnectTimout);
	}
}

void AuctionWorker::onReconnectTimer() {
	if(authSession.getState() != Stream::UnconnectedState || gameSession.getState() != Stream::UnconnectedState) {
		log(LL_Error, "Reconnection timed out, reconnecting...\n");
		if(authSession.getState() != Stream::UnconnectedState)
			authSession.abortSession();
		if(gameSession.getState() != Stream::UnconnectedState)
			gameSession.abortSession();
	} else {
		log(LL_Warning, "Reconnection timed out but neither auth or game connected, waiting...\n");
	}
}
