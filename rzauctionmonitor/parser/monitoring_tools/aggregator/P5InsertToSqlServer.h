#ifndef P5INSERTTOSQLSERVER_H
#define P5INSERTTOSQLSERVER_H

#include "AuctionFile.h"
#include "Core/Object.h"
#include "Database/DbQueryJobRef.h"
#include "IPipeline.h"
#include "P4ComputeStats.h"
#include <stdint.h>

struct DB_AuctionSummary;
template<class T> class DbQueryJob;

class P5InsertToSqlServer
    : public PipelineStep<std::tuple<std::string, time_t, AUCTION_FILE, std::vector<AUCTION_INFO_PER_DAY>>,
                          std::tuple<std::string, time_t, AUCTION_FILE>> {
	DECLARE_CLASSNAME(P5InsertToSqlServer, 0)
public:
	P5InsertToSqlServer();
	virtual void doWork(std::shared_ptr<WorkItem> item) override;

private:
	DbQueryJobRef dbQuery;
};

#endif
