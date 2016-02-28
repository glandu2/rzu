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
	auto it = auctionsState.find(uid);
	if(it != auctionsState.end()) {
		AuctionInfo& auctionInfo = it->second;
		if(auctionInfo.data.size() != (size_t)len ||
		   memcmp(auctionInfo.data.data(), data, len) != 0)
		{
			if(auctionInfo.flag != AuctionInfo::AIF_Added)
				auctionInfo.flag = AuctionInfo::AIF_Updated;

			auctionInfo.time = ::time(nullptr);
			auctionInfo.category = request->category;
			auctionInfo.page = request->page;
			auctionInfo.data = std::vector<uint8_t>(data, data + len);
			log(LL_Debug, "Auction info modified: %d\n", uid);
		} else if(auctionInfo.flag == AuctionInfo::AIF_NotProcessed) {
			auctionInfo.flag = AuctionInfo::AIF_Unmodifed;
		}
	} else {
		auctionsState.insert(std::make_pair(uid, AuctionInfo(uid, AuctionInfo::AIF_Added, request->category, request->page, data, len)));
		log(LL_Debug, "Auction info added: %d\n", uid);
	}

	auctionActives.push_back(AuctionInfo(uid, AuctionInfo::AIF_Added, request->category, request->page, data, len));
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
	currentCategory++;
	if(currentCategory > CATEGORY_MAX_INDEX) {
		dumpAuctions();

		currentCategory = 0;
	}

	totalPages = 1;
	addRequest(currentCategory, 1);
}

void AuctionManager::dumpAuctions()
{
	processRemovedAuctions(auctionsState);

	fileData.clear();
	serializeAuctionInfos(auctionsState, false, fileData);
	writeAuctionDataToFile(fileData, "_diff");

	postProcessAuctionInfos(auctionsState);

	fileData.clear();
	serializeAuctionInfos(auctionActives, true, fileData);
	writeAuctionDataToFile(fileData, "_full");
	auctionActives.clear();
}

void AuctionManager::processRemovedAuctions(std::unordered_map<uint32_t, AuctionInfo> &auctionInfos) {
	auto it = auctionInfos.begin();
	for(; it != auctionInfos.end(); ++it) {
		AuctionInfo& auctionInfo = it->second;
		if(auctionInfo.flag == AuctionInfo::AIF_NotProcessed) {
			auctionInfo.time = ::time(nullptr);
			log(LL_Debug, "Auction info removed: %d\n", auctionInfo.uid);
		}
	}
}

void AuctionManager::postProcessAuctionInfos(std::unordered_map<uint32_t, AuctionInfo> &auctionInfos) {
	auto it = auctionInfos.begin();
	for(; it != auctionInfos.end();) {
		AuctionInfo& auctionInfo = it->second;
		if(auctionInfo.flag == AuctionInfo::AIF_NotProcessed) {
			it = auctionInfos.erase(it);
		} else {
			auctionInfo.flag = AuctionInfo::AIF_NotProcessed;
			++it;
		}
	}
}

template<class Container>
void AuctionManager::serializeAuctionInfos(const Container &auctionInfos, bool includeUnmodified, std::vector<uint8_t> &output)
{
	bool memoryReserved = false;

	auto it = auctionInfos.begin();
	for(; it != auctionInfos.end(); ++it) {
		const AuctionInfo& auctionInfo = getAuctionInfoFromValue(*it);
		int diffType = getAuctionDiffType(auctionInfo.flag);

		if(!includeUnmodified && diffType == D_Unmodified)
			continue;

		Header header = {0};
		header.size = sizeof(header) + auctionInfo.data.size();
		header.version = 1;
		header.flag = (int16_t) diffType;
		header.time = auctionInfo.time;
		header.category = (int16_t) auctionInfo.category;

		if(!memoryReserved) {
			output.reserve(output.size() + header.size*auctionInfos.size());
			memoryReserved = true;
		}

		const char* p = (const char*)&header;
		output.insert(output.end(), p, p + sizeof(header));
		output.insert(output.end(), auctionInfo.data.begin(), auctionInfo.data.end());
	}
}

void AuctionManager::writeAuctionDataToFile(const std::vector<uint8_t> &data, const char* suffix)
{
	std::string auctionsDir = CONFIG_GET()->client.auctionListDir.get();
	std::string auctionsFile = CONFIG_GET()->client.auctionListFile.get();
	char filenameSuffix[256];
	struct tm localtm;

	Utils::getGmTime(time(NULL), &localtm);

	sprintf(filenameSuffix, "_%04d%02d%02d_%02d%02d%02d%s",
			localtm.tm_year, localtm.tm_mon, localtm.tm_mday,
			localtm.tm_hour, localtm.tm_min, localtm.tm_sec, suffix ? suffix : "");
	auctionsFile.insert(auctionsFile.find_first_of('.'), filenameSuffix);

	Utils::mkdir(auctionsDir.c_str());
	std::string auctionsFilename = auctionsDir + "/" + auctionsFile;

	FILE* file = fopen(auctionsFilename.c_str(), "wb");
	if(!file) {
		log(LL_Error, "Cannot open auction file %s\n", auctionsFilename.c_str());
		return;
	}

	size_t pos = auctionsFile.find_last_of(".gz");
	if(pos == (auctionsFile.size() - 1)) {
		log(LL_Info, "Writting compressed data to file %s\n", auctionsFile.c_str());

		std::vector<uint8_t> compressedData(compressBound((uLong)data.size()));
		uLongf compressedDataSize = (uLongf)compressedData.size();
		int result = compressGzip(&compressedData[0], &compressedDataSize, &data[0], (uLong)data.size(), Z_BEST_COMPRESSION);
		if(result == Z_OK) {
			if(fwrite(&compressedData[0], sizeof(uint8_t), compressedDataSize, file) != compressedDataSize) {
				log(LL_Error, "Failed to write data to file %s: error %d\n", auctionsFilename.c_str(), errno);
			}
		} else {
			log(LL_Error, "Failed to compress %d bytes\n", (int)data.size());
		}
	} else {
		log(LL_Info, "Writting data to file %s\n", auctionsFile.c_str());
		if(fwrite(&data[0], sizeof(uint8_t), data.size(), file) != data.size()) {
			log(LL_Error, "Failed to write data to file %s: error %d\n", auctionsFilename.c_str(), errno);
		}
	}

	fclose(file);
}

int AuctionManager::getAuctionDiffType(AuctionManager::AuctionInfo::Flag flag) {
	switch(flag) {
		case AuctionInfo::AIF_NotProcessed: return D_Deleted;
		case AuctionInfo::AIF_Added: return D_Added;
		case AuctionInfo::AIF_Updated: return D_Updated;
		case AuctionInfo::AIF_Unmodifed: return D_Unmodified;
	}
	return D_Unrecognized + flag;
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
