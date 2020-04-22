#pragma once

#include "./Extern.h"
#include "AuctionComplexDiffWriter.h"
#include "Config/ConfigParamVal.h"
#include "Core/Object.h"
#include "Core/Timer.h"
#include "IParser.h"
#include "NetSession/StartableObject.h"
#include <string>

struct AuctionFile {
	std::string filename;
	size_t alreadyExistingAuctions;
	size_t addedAuctionsInFile;
	AUCTION_SIMPLE_FILE auctions;
	bool isFull;

	AuctionFile() {
		isFull = true;
		alreadyExistingAuctions = 0;
		addedAuctionsInFile = 0;
	}

	static bool isFileFullType(const AUCTION_SIMPLE_FILE& auctions,
	                           AuctionComplexDiffWriter* auctionWriter,
	                           size_t* outAlreadyExistingAuctions = nullptr,
	                           size_t* outAddedAuctionsInFile = nullptr) {
		bool isFull = true;
		size_t alreadyExistingAuctions = 0;
		size_t addedAuctionsInFile = 0;

		for(size_t i = 0; i < auctions.auctions.size(); i++) {
			const AUCTION_SIMPLE_INFO& auctionData = auctions.auctions[i];
			if(auctionData.diffType != D_Added && auctionData.diffType != D_Base)
				isFull = false;
			if(auctionData.diffType == D_Added && auctionWriter->hasAuction(AuctionUid(auctionData.uid)))
				alreadyExistingAuctions++;
			if(auctionData.diffType == D_Added || auctionData.diffType == D_Base)
				addedAuctionsInFile++;
		}

		if(alreadyExistingAuctions == 0 && addedAuctionsInFile < auctionWriter->getAuctionCount() / 2)
			isFull = false;

		if(outAlreadyExistingAuctions)
			*outAlreadyExistingAuctions = alreadyExistingAuctions;

		if(outAddedAuctionsInFile)
			*outAddedAuctionsInFile = addedAuctionsInFile;

		return isFull;
	}

	void adjustDetectedType(AuctionComplexDiffWriter* auctionWriter) {
		isFull = isFileFullType(auctions, auctionWriter, &alreadyExistingAuctions, &addedAuctionsInFile);
	}
};

class RZAUCTIONWATCHER_EXTERN AuctionParser : public Object, public StartableObject {
	DECLARE_CLASSNAME(AuctionParser, 0)
public:
	AuctionParser(IParser* aggregator,
	              cval<std::string>& auctionsPath,
	              cval<int>& changeWaitSeconds,
	              cval<std::string>& statesPath,
	              cval<std::string>& auctionStateFile,
	              cval<std::string>& aggregationStateFile);

	virtual bool start();
	virtual void stop();
	virtual bool isStarted();

	bool importState();

protected:
	void onTimeout();
	static void onScandir(uv_fs_t* req);

	bool parseFile(std::string fullFilename);
	void exportState();

private:
	AuctionComplexDiffWriter auctionWriter;
	IParser* aggregator;
	cval<std::string>& auctionsPath;
	cval<int>& changeWaitSeconds;
	cval<std::string>& statesPath;
	cval<std::string>& auctionStateFile;
	cval<std::string>& aggregationStateFile;

	Timer<AuctionParser> dirWatchTimer;
	bool started;
	uv_fs_t scandirReq;
	std::string lastParsedFile;
};

