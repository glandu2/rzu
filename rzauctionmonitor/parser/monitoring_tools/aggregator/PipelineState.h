#pragma once

#include "AuctionFile.h"
#include <nonstd/optional.hpp>
#include <string>
#include <vector>

struct PipelineState {
	std::string associatedFilename;
	time_t timestamp;
};

struct PipelineAggregatedState {
	PipelineState base;
	std::vector<AUCTION_FILE> dumps;
};
