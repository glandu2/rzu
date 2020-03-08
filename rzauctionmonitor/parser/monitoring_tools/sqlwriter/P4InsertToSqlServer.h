#ifndef P4INSERTTOSQLSERVER_H
#define P4INSERTTOSQLSERVER_H

#include "AuctionFile.h"
#include "Core/Object.h"
#include "Database/DbQueryJobRef.h"
#include "IPipeline.h"
#include "PipelineState.h"
#include <stdint.h>

struct DB_AuctionSummary;
template<class T> class DbQueryJob;

class P4InsertToSqlServer : public PipelineStep<std::pair<PipelineState, AUCTION_FILE>, PipelineState> {
	DECLARE_CLASSNAME(P4InsertToSqlServer, 0)
public:
	P4InsertToSqlServer();
	virtual void doWork(std::shared_ptr<WorkItem> item) override;

private:
	DbQueryJobRef dbQuery;
};

#endif
