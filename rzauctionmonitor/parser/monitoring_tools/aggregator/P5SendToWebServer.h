#pragma once

#include "AuctionFile.h"
#include "Core/Object.h"
#include "HttpClientSession.h"
#include "IPipeline.h"
#include "PipelineState.h"
#include <stdint.h>

class P5SendToWebServer
    : public PipelineStep<std::pair<PipelineAggregatedState, std::string>, PipelineAggregatedState> {
	DECLARE_CLASSNAME(P5SendToWebServer, 0)
public:
	P5SendToWebServer(cval<std::string>& ip, cval<int>& port, cval<std::string>& url, cval<std::string>& pwd);
	virtual void doWork(std::shared_ptr<WorkItem> item) override;

private:
	HttpClientSession httpClientSession;
	cval<std::string>& url;
	cval<std::string>& pwd;
};

