#pragma once

#include "AuctionFile.h"
#include <nonstd/optional.hpp>
#include <string>

struct PipelineState {
	std::string lastFilenameParsed;
	nonstd::optional<AUCTION_FILE> fullDump;
};

