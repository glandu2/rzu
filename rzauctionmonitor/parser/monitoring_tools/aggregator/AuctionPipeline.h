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

#include "P1ReadAuction.h"
#include "P2ParseAuction.h"
#include "P3AggregateStats.h"
#include "P4ComputeStats.h"
#include "P5SendToWebServer.h"
#include "P6Commit.h"

/* scandir (disk, fast) => strings
 * readFile(disk) => AuctionComplexDiffWriter
 * parseAuctions (cpu) => reduce stats to json String
 * sendToHttp (network) */

class AuctionPipeline : public ICommitable<std::pair<std::string, std::string>>, public Object, public StartableObject {
	DECLARE_CLASSNAME(AuctionPipeline, 0)
public:
	AuctionPipeline(cval<std::string>& auctionsPath,
	                cval<int>& changeWaitSeconds,
	                cval<std::string>& statesPath,
	                cval<std::string>& auctionStateFile,
	                cval<std::string>& ip,
	                cval<int>& port,
	                cval<std::string>& url,
	                cval<std::string>& pwd);

	virtual bool start() override;
	virtual void stop() override;
	virtual bool isStarted() override;

	virtual void commit(std::unique_ptr<IPipelineOutput<std::pair<std::string, std::string>>> item,
	                    int status) override;

	void importState();
	int exportState(const std::string& fullFilename, time_t timestamp, const AUCTION_FILE& auctionFile);

protected:
	void onTimeout();
	static void onScandir(uv_fs_t* req);

private:
	cval<std::string>& auctionsPath;
	cval<int>& changeWaitSeconds;
	cval<std::string>& statesPath;
	cval<std::string>& auctionStateFile;

	Timer<AuctionPipeline> dirWatchTimer;
	bool started;
	uv_fs_t scandirReq;
	std::string lastQueuedFile;

	P1ReadAuction readAuctionStep;
	P2ParseAuction parseAuctionStep;
	P3AggregateStats aggregateStatsStep;
	P4ComputeStats computeStatsStep;
	P5SendToWebServer sendToWebServerStep;
	P6Commit commitStep;
};

#endif
