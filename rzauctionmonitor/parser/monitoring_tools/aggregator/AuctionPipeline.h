#ifndef AUCTIONPIPELINE_H
#define AUCTIONPIPELINE_H

#include "AuctionComplexDiffWriter.h"
#include "Config/ConfigParamVal.h"
#include "Core/Object.h"
#include "Core/Timer.h"
#include "IPipeline.h"
#include "NetSession/StartableObject.h"
#include <stdint.h>
#include <string>
#include <uv.h>
#include <vector>

#include "P0ScanAuctions.h"
#include "P12DeserializeAuction.h"
#include "P1ReadAuction.h"
#include "P2ParseAuction.h"
#include "P3AggregateStats.h"
#include "P4ComputeStats.h"
#include "P5InsertToSqlServer.h"
#include "P5SendToWebServer.h"
#include "P6Commit.h"

/* scandir (disk, fast) => strings
 * readFile(disk) => AuctionComplexDiffWriter
 * parseAuctions (cpu) => reduce stats to json String
 * sendToHttp (network) */

class AuctionPipeline : public Object, public StartableObject {
	DECLARE_CLASSNAME(AuctionPipeline, 0)
public:
	AuctionPipeline(cval<std::string>& auctionsPath,
	                cval<int>& changeWaitSeconds,
	                cval<std::string>& statesPath,
	                cval<std::string>& auctionStateFile);

	virtual bool start() override;
	virtual void stop() override;
	virtual bool isStarted() override;

	void importState();
	int exportState(std::string& fullFilename, AUCTION_FILE& auctionFile);

protected:
	void onTimeout();
	static void onScandir(uv_fs_t* req);
	static void onFsEvent(uv_fs_event_t* handle, const char* filename, int events, int status);

private:
	cval<std::string>& auctionsPath;
	cval<int>& changeWaitSeconds;
	cval<std::string>& statesPath;
	cval<std::string>& auctionStateFile;

	Timer<AuctionPipeline> dirWatchTimer;
	bool started;
	uv_fs_t scandirReq;
	uv_fs_event_t fsEvent;
	std::string lastQueuedFile;

	P0ScanAuctions scanAuctionStep;
	P1ReadAuction readAuctionStep;
	P12DeserializeAuction deserializeAuctionStep;
	P2ParseAuction parseAuctionStep;
	P3AggregateStats aggregateStatsStep;
	P4ComputeStats computeStatsStep;
	P5InsertToSqlServer sendToWebServerStep;
	P6Commit commitStep;
};

#endif
