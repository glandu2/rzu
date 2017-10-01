#ifndef AUCTIONPARSER_H
#define AUCTIONPARSER_H

#include "./Extern.h"
#include "AuctionComplexDiffWriter.h"
#include "Core/Object.h"
#include "Core/Timer.h"
#include "IParser.h"
#include "NetSession/StartableObject.h"
#include <string>

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

#endif
