#ifndef PIPELINESTATE_H
#define PIPELINESTATE_H

#include "AuctionFile.h"
#include <nonstd/optional.hpp>
#include <string>
#include <vector>

struct PipelineState {
	std::string lastFilenameParsed;
	nonstd::optional<AUCTION_FILE> fullDump;
	time_t timestamp;
};

struct PipelineAggregatedState {
	std::string lastFilenameParsed;
	std::vector<AUCTION_FILE> dumps;
	time_t timestamp;

	void reset() {
		this->lastFilenameParsed.clear();
		this->dumps.clear();
		this->timestamp = 0;
	}
};

#endif
