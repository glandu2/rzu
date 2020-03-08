#ifndef PIPELINESTATE_H
#define PIPELINESTATE_H

#include "AuctionFile.h"
#include <nonstd/optional.hpp>
#include <string>
#include <vector>

struct PipelineState {
	std::string lastFilenameParsed;
	time_t timestamp;
};

struct PipelineAggregatedState {
	PipelineState base;
	std::vector<AUCTION_FILE> dumps;
};

#endif
