#include "AuctionManager.h"
#include "Console/ConsoleCommands.h"
#include "Core/PrintfFormats.h"
#include "GlobalConfig.h"
#include <iterator>
#include <stdio.h>
#include <stdlib.h>

AuctionManager* AuctionManager::instance = nullptr;

void AuctionManager::onReloadAccounts(IWritableConsole* console, const std::vector<std::string>&) {
	if(!instance) {
		console->write("No auction manager instance !\r\n");
	} else if(!instance->stoppingClients.empty() || instance->reloadingAccounts) {
		console->write("Account reload already in progress\r\n");
	} else {
		instance->reloadAccounts();
		console->write("Reloading accounts\r\n");
	}
}

AuctionManager::AuctionManager()
    : auctionWriter(CATEGORY_MAX_INDEX), stopped(true), totalPages(0), firstDump(true), reloadingAccounts(false) {
	if(!instance) {
		ConsoleCommands::get()->addCommand("client.reload_accounts",
		                                   "reload",
		                                   0,
		                                   0,
		                                   &AuctionManager::onReloadAccounts,
		                                   "Restart clients and reload accounts from accounts file");
		instance = this;
	} else {
		log(LL_Error, "Multiples instance of AuctionManager started, reload command will not apply to this instance\n");
	}
}

bool AuctionManager::start() {
	loadInitialState();
	loadAccounts();

	stopped = false;
	pendingRequests.clear();
	totalPages = 1;
	currentCategory = 0;
	addRequest(currentCategory, 1);

	return true;
}

void AuctionManager::stop() {
	stopped = true;
	stopClients();
}

void AuctionManager::stopClients() {
	log(LL_Info, "Stopping %d clients\n", (int) clients.size());

	stoppingClients.insert(
	    stoppingClients.end(), std::make_move_iterator(clients.begin()), std::make_move_iterator(clients.end()));
	clients.clear();

	auto it = stoppingClients.begin();
	for(; it != stoppingClients.end(); ++it) {
		(*it)->stop(Callback<AuctionWorker::StoppedCallback>(this, &AuctionManager::onClientStoppedStatic));
	}
}

bool AuctionManager::isStarted() {
	return !clients.empty();
}

void AuctionManager::reloadAccounts() {
	if(stoppingClients.empty()) {
		reloadingAccounts = true;
		stopClients();
		if(stoppingClients.empty()) {
			loadAccounts();
		}
	} else {
		log(LL_Error, "Attempt to reload clients but %d clients are still stopping\n", (int) stoppingClients.size());
	}
}

void AuctionManager::loadAccounts() {
	std::string accountFile = CONFIG_GET()->client.accountFile.get();
	cval<std::string>& ip = CONFIG_GET()->client.ip;
	cval<int>& port = CONFIG_GET()->client.port;
	cval<int>& serverIdx = CONFIG_GET()->client.gsindex;
	cval<int>& recoDelay = CONFIG_GET()->client.recoDelay;
	cval<int>& autoRecoDelay = CONFIG_GET()->client.autoRecoDelay;
	cval<bool>& useRsa = CONFIG_GET()->client.useRsa;

	char line[1024];
	char *p, *lastP;
	int lineNumber = 0;

	std::unique_ptr<FILE, int (*)(FILE*)> file(fopen(accountFile.c_str(), "rt"), &fclose);
	if(!file) {
		log(LL_Error, "Cannot find account file %s\n", accountFile.c_str());
		return;
	}

	while(fgets(line, sizeof(line), file.get()) != NULL) {
		std::string account, password, playerName;

		lineNumber++;

		p = strchr(line, '\t');
		if(!p) {
			log(LL_Warning,
			    "Error in account file %s:%d: missing tabulation separator after account name\n",
			    accountFile.c_str(),
			    lineNumber);
			continue;
		}
		account = std::string(line, p - line);
		if(account.size() == 0 || account[0] == '#')
			continue;

		lastP = p + 1;
		p = strchr(lastP, '\t');
		if(!p) {
			log(LL_Warning,
			    "Error in account file %s:%d: missing tabulation separator after password\n",
			    accountFile.c_str(),
			    lineNumber);
			continue;
		}
		password = std::string(lastP, p - lastP);

		lastP = p + 1;
		p = strpbrk(lastP, "\r\n");
		if(!p) {
			playerName = std::string(lastP);
		} else {
			playerName = std::string(lastP, p - lastP);
		}

		AuctionWorker* auctionWorker = new AuctionWorker(
		    this, ip, port, serverIdx, recoDelay, useRsa, autoRecoDelay, account, password, playerName);
		auctionWorker->start();
		clients.push_back(std::unique_ptr<AuctionWorker>(auctionWorker));

		log(LL_Info, "Added account %s:%s:%s\n", account.c_str(), password.c_str(), playerName.c_str());
	}
	log(LL_Info, "Loaded %d accounts\n", (int) clients.size());
}

void AuctionManager::onClientStoppedStatic(IListener* instance, AuctionWorker* worker) {
	((AuctionManager*) instance)->onClientStopped(worker);
}
void AuctionManager::onClientStopped(AuctionWorker* worker) {
	bool foundWorker = false;

	log(LL_Info, "Worker stopped: %s\n", worker->getObjectName());

	auto it = stoppingClients.begin();
	for(; it != stoppingClients.end(); ++it) {
		std::unique_ptr<AuctionWorker>& currentWorker = *it;
		if(currentWorker.get() == worker) {
			AuctionWorker* workerPtr = currentWorker.release();
			workerPtr->deleteLater();
			stoppingClients.erase(it);
			foundWorker = true;
			break;
		}
	}

	if(!foundWorker)
		log(LL_Warning,
		    "Worker %s stopped but not found in currently stopping workers, %d worker left to stop\n",
		    worker->getObjectName(),
		    (int) stoppingClients.size());

	if(stoppingClients.empty() && !stopped)
		accountReloadTimer.start(this, &AuctionManager::onAccountReloadTimer, CONFIG_GET()->client.recoDelay.get(), 0);
}

void AuctionManager::onAccountReloadTimer() {
	loadAccounts();
	reloadingAccounts = false;
}

void AuctionManager::addRequest(int category, int page) {
	log(LL_Debug, "Adding request for category %d, page %d\n", category, page);

	AuctionWorker::AuctionRequest* request = new AuctionWorker::AuctionRequest(category, page);
	pendingRequests.push_back(std::unique_ptr<AuctionWorker::AuctionRequest>(request));

	for(size_t i = 0; i < clients.size(); i++) {
		if(!clients[i]->hasRequest())
			clients[i]->wakeUp();
	}
}

std::unique_ptr<AuctionWorker::AuctionRequest> AuctionManager::getNextRequest() {
	if(pendingRequests.empty()) {
		log(LL_Debug, "No more pending request\n");
		return std::unique_ptr<AuctionWorker::AuctionRequest>();
	} else {
		std::unique_ptr<AuctionWorker::AuctionRequest> ret = std::move(pendingRequests.front());

		log(LL_Debug, "Poping next request: category %d, page %d\n", ret->category, ret->page);

		if(ret->page == 1 && ret->category == (int) currentCategory) {
			auctionWriter.beginCategory(ret->category, ::time(nullptr));
		} else if(ret->page == 1) {
			log(LL_Warning,
			    "Next request is page 1 of category %d but current category is %d\n",
			    ret->category,
			    currentCategory);
		}

		pendingRequests.pop_front();
		return ret;
	}
}

void AuctionManager::addAuctionInfo(const AuctionWorker::AuctionRequest* request,
                                    uint32_t uid,
                                    const uint8_t* data,
                                    int len) {
	auctionWriter.addAuctionInfo(AuctionUid(uid), time(nullptr), request->category, data, len);
}

void AuctionManager::onAuctionSearchCompleted(bool success,
                                              int pageTotal,
                                              std::unique_ptr<AuctionWorker::AuctionRequest> request) {
	log(LL_Debug, "Received search result: %s\n", success ? "success" : "failure");
	if(!request) {
		log(LL_Warning, "Search result for null request\n");
	} else if(!success) {
		log(LL_Warning, "Request for category %d, page %d failed, retrying\n", request->category, request->page);
		pendingRequests.push_front(std::move(request));
	} else {
		// load new pages
		for(int page = totalPages + 1; page <= pageTotal; page++) {
			addRequest(request->category, page);
		}
		if(pageTotal > totalPages)
			totalPages = pageTotal;
		else if(isAllRequestProcessed()) {
			onAllRequestProcessed();
		}
	}
}

bool AuctionManager::isAllRequestProcessed() {
	if(!pendingRequests.empty())
		return false;

	for(size_t i = 0; i < clients.size(); i++) {
		if(clients[i]->hasRequest())
			return false;
	}

	return true;
}

void AuctionManager::dumpAuctions() {
	time_t dumpTimeStamp = auctionWriter.getLastEndCategoryTime();
	if(dumpTimeStamp == 0) {
		log(LL_Warning, "Last category end timestamp is 0, using current timestamp\n");
		dumpTimeStamp = ::time(nullptr);
	}

	if(firstDump)
		firstDumpTime = dumpTimeStamp;

	const std::string& auctionDir = CONFIG_GET()->client.auctionListDir.get();
	const std::string& auctionFile = CONFIG_GET()->client.auctionListFile.get();
	const std::string& auctionState = CONFIG_GET()->client.stateFile.get();

	if(firstDump || CONFIG_GET()->client.doFullAuctionDump.get()) {
		auctionWriter.dumpAuctions(fileData, true);
		auctionWriter.writeAuctionDataToFile(auctionDir, auctionFile, fileData, dumpTimeStamp, "_full");
	}

	if(CONFIG_GET()->client.doStateAuctionDump.get()) {
		auctionWriter.dumpAuctions(fileData, true);
		auctionWriter.writeAuctionDataToFile(auctionDir, auctionState, fileData);
	}

	if(!firstDump) {
		auctionWriter.dumpAuctions(fileData, false);
		auctionWriter.writeAuctionDataToFile(auctionDir, auctionFile, fileData, dumpTimeStamp, "_diff");
	}
}

void AuctionManager::loadInitialState() {
	struct Header {
		char sign[4];
		uint32_t version;
	};
	std::vector<uint8_t> buffer;
	std::string filename = CONFIG_GET()->client.stateFile.get();

	if(filename.empty()) {
		log(LL_Info, "No auction initial state file to load\n");
		return;
	}

	auctionWriter.readAuctionDataFromFile(CONFIG_GET()->client.auctionListDir.get(), filename, buffer);

	uint32_t version = ((Header*) buffer.data())->version;

	MessageBuffer structBuffer(buffer.data(), buffer.size(), version);

	AUCTION_SIMPLE_FILE auctionFile;
	auctionFile.deserialize(&structBuffer);
	if(!structBuffer.checkFinalSize()) {
		log(LL_Error, "Invalid file data, can't deserialize\n");
		return;
	}

	if(auctionFile.header.dumpType != DT_Full) {
		log(LL_Error, "Can't import a non full dump\n");
		return;
	}

	auctionWriter.importDump(&auctionFile);
}

void AuctionManager::onAllRequestProcessed() {
	auctionWriter.endCategory(currentCategory, ::time(nullptr));

	currentCategory++;
	if(currentCategory >= CATEGORY_MAX_INDEX) {
		auctionWriter.endProcess();

		dumpAuctions();

		auctionWriter.beginProcess();
		currentCategory = 0;
		firstDump = false;
	}

	totalPages = 1;
	addRequest(currentCategory, 1);
}
