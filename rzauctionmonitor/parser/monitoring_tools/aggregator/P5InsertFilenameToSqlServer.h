#pragma once

#include "AuctionFile.h"
#include "Core/Object.h"
#include "Database/DbQueryJobRef.h"
#include "IPipeline.h"
#include "P4ComputeStats.h"
#include "PipelineState.h"
#include <stdint.h>

template<class T> class DbQueryJob;

struct DB_InsertFilename {
	static cval<std::string>& connectionString;
	struct Input {
		std::string filename;
	};

	struct Output {
		int32_t uid;
	};
};

class P5InsertFilenameToSqlServer
    : public PipelineStep<std::pair<PipelineAggregatedState, std::vector<AUCTION_INFO_PER_DAY>>,
                          std::pair<PipelineAggregatedState, std::vector<AUCTION_INFO_PER_DAY>>> {
	DECLARE_CLASSNAME(P5InsertFilenameToSqlServer, 0)
public:
	P5InsertFilenameToSqlServer();
	virtual void doWork(std::shared_ptr<WorkItem> item) override;

private:
	DbQueryJobRef dbQuery;
};
