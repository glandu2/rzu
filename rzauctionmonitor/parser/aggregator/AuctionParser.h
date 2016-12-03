#ifndef AUCTIONPARSER_H
#define AUCTIONPARSER_H

#include "Core/Object.h"
#include "AuctionComplexDiffWriter.h"
#include "Core/Timer.h"
#include <string>
#include "Aggregator.h"
#include "NetSession/StartableObject.h"

class AuctionParser : public Object, public StartableObject
{
	DECLARE_CLASSNAME(AuctionParser, 0)
public:
	AuctionParser();

	virtual bool start();
	virtual void stop();
	virtual bool isStarted();

	bool importState();

protected:
	void onTimeout();
	static void onScandir(uv_fs_t* req);

	bool parseFile(const char* filename);
	void exportState();

private:
	AuctionComplexDiffWriter auctionWriter;
	Aggregator aggregator;
	cval<std::string>& auctionsPath;
	cval<std::string>& statesPath;
	cval<std::string>& auctionStateFile;
	cval<std::string>& aggregationStateFile;


	Timer<AuctionParser> dirWatchTimer;
	bool started;
	uv_fs_t scandirReq;
	std::string lastParsedFile;
};

#endif
