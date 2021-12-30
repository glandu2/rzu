#pragma once

#include "AuctionFile.h"
#include "Core/Object.h"
#include "Database/DbQueryJobRef.h"
#include "IPipeline.h"
#include "PipelineState.h"
#include <stdint.h>

template<class T> class DbQueryJob;

struct DB_InsertFilename {
	struct Input {
		std::string filename;
		DbDateTime start_time;
	};

	struct Output {
		int32_t uid;
	};
};

class P5InsertFilenameToSqlServer
    : public PipelineStep<std::pair<PipelineState, AUCTION_FILE>, std::pair<PipelineState, AUCTION_FILE>> {
	DECLARE_CLASSNAME(P5InsertFilenameToSqlServer, 0)
public:
	P5InsertFilenameToSqlServer();
	virtual void doWork(std::shared_ptr<WorkItem> item) override;

private:
	DbQueryJobRef dbQuery;
};
