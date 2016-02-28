#include "AuctionManager.h"
#include "GlobalConfig.h"
#include <stdio.h>
#include <stdlib.h>
#include "zlib.h"

static int compressGzip(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen, int level);

AuctionManager::AuctionManager() : totalPages(0)
{

}

void AuctionManager::start()
{
	std::string accountFile = CONFIG_GET()->client.accountFile.get();
	std::string ip = CONFIG_GET()->client.ip.get();
	uint16_t port = (uint16_t) CONFIG_GET()->client.port.get();
	int serverIdx = CONFIG_GET()->client.gsindex.get();
	int recoDelay = CONFIG_GET()->client.recoDelay.get();
	int autoRecoDelay = CONFIG_GET()->client.autoRecoDelay.get();
	bool useRsa = CONFIG_GET()->client.useRsa.get();

	char line[1024];
	char *p, *lastP;
	int lineNumber = 0;

	FILE* file = fopen(accountFile.c_str(), "rt");
	if(!file) {
		log(LL_Error, "Cannot find account file %s\n", accountFile.c_str());
		return;
	}

	while (fgets(line, sizeof(line), file) != NULL)  {
		std::string account, password, playerName;

		lineNumber++;

		p = strchr(line, '\t');
		if(!p) {
			log(LL_Warning, "Error in account file %s:%d: missing tabulation separator after account name\n", accountFile.c_str(), lineNumber);
			continue;
		}
		account = std::string(line, p - line);
		if(account.size() == 0 || account[0] == '#')
			continue;

		lastP = p+1;
		p = strchr(lastP, '\t');
		if(!p) {
			log(LL_Warning, "Error in account file %s:%d: missing tabulation separator after password\n", accountFile.c_str(), lineNumber);
			continue;
		}
		password = std::string(lastP, p - lastP);

		lastP = p+1;
		p = strpbrk(lastP, "\r\n");
		if(!p) {
			playerName = std::string(lastP);
		} else {
			playerName = std::string(lastP, p - lastP);
		}

		AuctionWorker* auctionWorker = new AuctionWorker(this,
														 playerName,
														 ip,
														 port,
														 account,
														 password,
														 serverIdx,
														 recoDelay,
														 autoRecoDelay,
														 useRsa);
		auctionWorker->start();
		clients.push_back(std::unique_ptr<AuctionWorker>(auctionWorker));

		log(LL_Info, "Added account %s:%s:%s\n", account.c_str(), password.c_str(), playerName.c_str());
	}
	log(LL_Info, "Loaded %d accounts\n", (int)clients.size());

	totalPages = 1;
	currentCategory = 0;
	addRequest(currentCategory, 1);
}

void AuctionManager::stop()
{
//	for(int i = 0; i < clients.size(); i++) {
//		clients[i]->stop();
//	}
	pendingRequests.clear();
}

void AuctionManager::reloadAccounts() {

}

void AuctionManager::onClientStopped(IListener* instance) {
	AuctionWorker* worker = (AuctionWorker*) instance;
	bool foundWorker = false;

	log(LL_Info, "Worker stopped: %s\n", worker->getObjectName());

	auto it = stoppingClients.begin();
	for(; it != stoppingClients.end(); ++it) {
		std::unique_ptr<AuctionWorker>& currentWorker = *it;
		if(currentWorker.get() == worker) {
			stoppingClients.erase(it);
			foundWorker = true;
			break;
		}
	}

	if(!foundWorker)
		log(LL_Warning, "Worker %s stopped but not found in currently stopping workers, %d worker left to stop\n", worker->getObjectName(), (int)stoppingClients.size());
}

void AuctionManager::addRequest(int category, int page)
{
	log(LL_Debug, "Adding request for category %d, page %d\n", category, page);

	AuctionWorker::AuctionRequest* request = new AuctionWorker::AuctionRequest(category, page);
	pendingRequests.push_back(std::unique_ptr<AuctionWorker::AuctionRequest>(request));

	for(size_t i = 0; i < clients.size(); i++) {
		if(!clients[i]->hasRequest())
			clients[i]->wakeUp();
	}
}

void AuctionManager::addAuctionInfo(const AuctionWorker::AuctionRequest *request, uint32_t uid, const char *data, int len)
{
	auctionInfos.push_back(AuctionInfo(uid, request->category, request->page, data, len));
}

void AuctionManager::onAuctionSearchCompleted(bool success, int pageTotal, std::unique_ptr<AuctionWorker::AuctionRequest> request)
{
	log(LL_Debug, "Received search result: %s\n", success ? "success" : "failure");
	if(!request) {
		log(LL_Warning, "Search result for null request\n");
	} else if(!success) {
		request->failureNumber++;
		if(request->failureNumber < 3)
			pendingRequests.push_front(std::move(request));
		else
			log(LL_Warning, "Request for category %d, page %d failed %d times, giving up\n", request->category, request->page, request->failureNumber);
	} else {
		//load new pages
		for(int page = totalPages+1; page <= pageTotal; page++) {
			addRequest(request->category, page);
		}
		if(pageTotal > totalPages)
			totalPages = pageTotal;
		else if(isAllRequestProcessed()) {
			onAllRequestProcessed();
		}
	}
}

std::unique_ptr<AuctionWorker::AuctionRequest> AuctionManager::getNextRequest()
{
	if(pendingRequests.empty()) {
		log(LL_Debug, "No more pending request\n");
		return std::unique_ptr<AuctionWorker::AuctionRequest>();
	} else {
		std::unique_ptr<AuctionWorker::AuctionRequest> ret = std::move(pendingRequests.front());

		log(LL_Debug, "Poping next request: category %d, page %d\n", ret->category, ret->page);

		pendingRequests.pop_front();
		return ret;
	}
}

bool AuctionManager::isAllRequestProcessed()
{
	if(!pendingRequests.empty())
		return false;

	for(size_t i = 0; i < clients.size(); i++) {
		if(clients[i]->hasRequest())
			return false;
	}

	return true;
}

void AuctionManager::onAllRequestProcessed()
{
	log(LL_Info, "Auction results: category %d, %d pages, %d items\n", currentCategory, totalPages, (int)auctionInfos.size());
	dumpAuctions();

	auctionInfos.clear();
	if(currentCategory < 18)
		currentCategory++;
	else
		currentCategory = 0;

	totalPages = 1;
	addRequest(currentCategory, 1);
}

bool AuctionManager::dumpAuctions()
{
	bool success = true;
	std::string auctionsDir = CONFIG_GET()->client.auctionListDir.get();
	std::string auctionsFile = CONFIG_GET()->client.auctionListFile.get();
	char filenameSuffix[256];
	struct tm localtm;

	Utils::getGmTime(time(NULL), &localtm);

	sprintf(filenameSuffix, "_%02d_%04d%02d%02d_%02d%02d%02d", currentCategory,
			localtm.tm_year, localtm.tm_mon, localtm.tm_mday,
			localtm.tm_hour, localtm.tm_min, localtm.tm_sec);
	auctionsFile.insert(auctionsFile.find_first_of('.'), filenameSuffix);

	Utils::mkdir(auctionsDir.c_str());
	std::string auctionsFilename = auctionsDir + "/" + auctionsFile;

	FILE* file = fopen(auctionsFilename.c_str(), "wb");
	if(!file) {
		log(LL_Error, "Cannot open auction file %s\n", auctionsFilename.c_str());
		return false;
	}

	std::vector<uint8_t> fileData;
	auto it = auctionInfos.begin();
	for(; it != auctionInfos.end(); ++it) {
		const AuctionInfo& auctionInfo = *it;

		uint32_t size = uint32_t(sizeof(auctionInfo.time) +
								 sizeof(auctionInfo.category) +
								 sizeof(auctionInfo.page) +
								 auctionInfo.data.size());

		if(fileData.empty())
			fileData.reserve(size*auctionInfos.size());

		static_assert(sizeof(size) == 4, "sizeof(size) must be 4");
		static_assert(sizeof(auctionInfo.time) == 8, "sizeof(auctionInfo.time) must be 8");
		static_assert(sizeof(auctionInfo.category) == 4, "sizeof(auctionInfo.category) must be 4");
		static_assert(sizeof(auctionInfo.page) == 4, "sizeof(auctionInfo.page) must be 4");

		fileData.push_back(size & 0xFF);
		fileData.push_back((size >> 8) & 0xFF);
		fileData.push_back((size >> 16) & 0xFF);
		fileData.push_back((size >> 24) & 0xFF);

		fileData.push_back(auctionInfo.time & 0xFF);
		fileData.push_back((auctionInfo.time >> 8) & 0xFF);
		fileData.push_back((auctionInfo.time >> 16) & 0xFF);
		fileData.push_back((auctionInfo.time >> 24) & 0xFF);
		fileData.push_back((auctionInfo.time >> 32) & 0xFF);
		fileData.push_back((auctionInfo.time >> 40) & 0xFF);
		fileData.push_back((auctionInfo.time >> 48) & 0xFF);
		fileData.push_back((auctionInfo.time >> 56) & 0xFF);

		fileData.push_back(auctionInfo.category & 0xFF);
		fileData.push_back((auctionInfo.category >> 8) & 0xFF);
		fileData.push_back((auctionInfo.category >> 16) & 0xFF);
		fileData.push_back((auctionInfo.category >> 24) & 0xFF);

		fileData.push_back(auctionInfo.page & 0xFF);
		fileData.push_back((auctionInfo.page >> 8) & 0xFF);
		fileData.push_back((auctionInfo.page >> 16) & 0xFF);
		fileData.push_back((auctionInfo.page >> 24) & 0xFF);


		fileData.insert(fileData.end(), auctionInfo.data.begin(), auctionInfo.data.end());
	}

	size_t pos = auctionsFile.find_last_of(".gz");
	if(pos == (auctionsFile.size() - 1)) {
		log(LL_Info, "Writting compressed data to file %s\n", auctionsFile.c_str());

		std::vector<uint8_t> compressedData(compressBound((uLong)fileData.size()));
		uLongf compressedDataSize = (uLongf)compressedData.size();
		int result = compressGzip(&compressedData[0], &compressedDataSize, &fileData[0], (uLong)fileData.size(), Z_BEST_COMPRESSION);
		if(result == Z_OK) {
			if(fwrite(&compressedData[0], sizeof(uint8_t), compressedDataSize, file) != compressedDataSize) {
				log(LL_Error, "Failed to write data to file %s: error %d\n", auctionsFilename.c_str(), errno);
			}
		} else {
			log(LL_Error, "Failed to compress %d bytes\n", (int)fileData.size());
		}
	} else {
		log(LL_Info, "Writting data to file %s\n", auctionsFile.c_str());
		if(fwrite(&fileData[0], sizeof(uint8_t), fileData.size(), file) != fileData.size()) {
			log(LL_Error, "Failed to write data to file %s: error %d\n", auctionsFilename.c_str(), errno);
		}
	}

	fclose(file);
	return success;
}

static int compressGzip(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen, int level) {
	z_stream stream;
	int err;

	stream.next_in = (z_const Bytef *)source;
	stream.avail_in = (uInt)sourceLen;
	stream.next_out = dest;
	stream.avail_out = (uInt)*destLen;
	if ((uLong)stream.avail_out != *destLen)
		return Z_BUF_ERROR;

	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;
	stream.opaque = (voidpf)0;

	err = deflateInit2(&stream, level, Z_DEFLATED, MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY);
	if (err != Z_OK) return err;

	err = deflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END) {
		deflateEnd(&stream);
		return err == Z_OK ? Z_BUF_ERROR : err;
	}
	*destLen = stream.total_out;

	err = deflateEnd(&stream);
	return err;
}
