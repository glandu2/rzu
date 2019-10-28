#include "P5SendToWebServer.h"
#include "GlobalConfig.h"

P5SendToWebServer::P5SendToWebServer(cval<std::string>& ip,
                                     cval<int>& port,
                                     cval<std::string>& url,
                                     cval<std::string>& pwd)
    : PipelineStep<std::tuple<std::string, time_t, AUCTION_FILE, std::string>,
                   std::tuple<std::string, time_t, AUCTION_FILE>>(10, 2),
      httpClientSession(ip, port),
      url(url),
      pwd(pwd) {}

void P5SendToWebServer::doWork(std::shared_ptr<PipelineStep::WorkItem> item) {
	std::tuple<std::string, time_t, AUCTION_FILE, std::string> jsonData = std::move(item->getSource());
	std::string fullUrl;
	struct tm currentDateTm;

	item->setName(std::to_string(std::get<1>(jsonData)));

	Utils::getGmTime(std::get<1>(jsonData), &currentDateTm);

	Utils::stringFormat(fullUrl,
	                    "%s/%04d%02d%02d/%s",
	                    url.get().c_str(),
	                    currentDateTm.tm_year,
	                    currentDateTm.tm_mon,
	                    currentDateTm.tm_mday,
	                    pwd.get().c_str());

	log(LL_Info, "Sending data to url %s\n", fullUrl.c_str());

	addResult(
	    item,
	    std::make_tuple(
	        std::move(std::get<0>(jsonData)), std::move(std::get<1>(jsonData)), std::move(std::get<2>(jsonData))));

	// httpClientSession.sendData(fullUrl, std::get<3>(jsonData), [this, item]() { workDone(item, 0); });
	workDone(item, 0);
}
