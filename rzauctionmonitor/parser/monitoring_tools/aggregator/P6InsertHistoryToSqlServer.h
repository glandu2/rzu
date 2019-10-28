#ifndef P4INSERTTOSQLSERVER_H
#define P4INSERTTOSQLSERVER_H

#include "AuctionFile.h"
#include "Core/Object.h"
#include "Database/DbQueryJobRef.h"
#include "IPipeline.h"
#include "PipelineState.h"
#include <stdint.h>

template<class T> class DbQueryJob;

class P6InsertHistoryToSqlServer : public PipelineStep<PipelineAggregatedState, PipelineAggregatedState> {
	DECLARE_CLASSNAME(P6InsertHistoryToSqlServer, 0)
public:
	P6InsertHistoryToSqlServer();
	virtual void doWork(std::shared_ptr<WorkItem> item) override;

private:
	DbQueryJobRef dbQuery;
};

#endif
