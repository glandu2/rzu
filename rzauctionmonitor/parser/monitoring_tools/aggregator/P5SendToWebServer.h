#ifndef P5SENDTOWEBSERVER_H
#define P5SENDTOWEBSERVER_H

#include "AuctionFile.h"
#include "Core/Object.h"
#include "HttpClientSession.h"
#include "IPipeline.h"
#include <stdint.h>

class P5SendToWebServer : public PipelineStep<std::tuple<std::string, time_t, AUCTION_FILE, std::string>,
                                              std::tuple<std::string, time_t, AUCTION_FILE>> {
	DECLARE_CLASSNAME(P5SendToWebServer, 0)
public:
	P5SendToWebServer(cval<std::string>& ip, cval<int>& port, cval<std::string>& url, cval<std::string>& pwd);
	virtual void doWork(std::shared_ptr<WorkItem> item) override;

private:
	HttpClientSession httpClientSession;
	cval<std::string>& url;
	cval<std::string>& pwd;
};

#endif
