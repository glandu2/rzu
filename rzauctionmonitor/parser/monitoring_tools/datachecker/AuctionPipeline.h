#pragma once

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
#include "P2DeserializeAuction.h"

/* scandir (disk, fast) => strings
 * readFile(disk) => AuctionComplexDiffWriter
 * parseAuctions (cpu) => reduce stats to json String
 * sendToHttp (network) */

class AuctionPipeline : public Object,
                        public StartableObject,
                        public IPipelineProducer<std::pair<std::string, std::string>> {
	DECLARE_CLASSNAME(AuctionPipeline, 0)
public:
	AuctionPipeline(cval<std::string>& auctionsPath, cval<int>& changeWaitSeconds, cval<std::string>& lastQueuedFile);

	virtual bool start() override;
	virtual void stop() override;
	virtual bool isStarted() override;

	virtual void notifyError(int status) override;
	virtual void notifyOutputAvailable() override;

protected:
	void onTimeout();
	static void onScandir(uv_fs_t* req);
	static void onFsEvent(uv_fs_event_t* handle, const char* filename, int events, int status);

private:
	cval<std::string>& auctionsPath;
	cval<int>& changeWaitSeconds;
	cval<std::string>& lastQueuedFile;

	Timer<AuctionPipeline> dirWatchTimer;
	bool started;
	uv_fs_t scandirReq;
	uv_fs_event_t fsEvent;

	P1ReadAuction readAuctionStep;
	P2DeserializeAuction deserializeAuctionStep;
};
