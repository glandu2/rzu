#pragma once

#include <string>

class AuctionComplexDiffWriter;

class IParser {
public:
	virtual bool isFull() = 0;

	virtual bool parseAuctions(AuctionComplexDiffWriter* auctionWriter) = 0;

	virtual void exportState(std::string filename, const std::string& lastParsedFile) = 0;
	virtual void importState(std::string filename, std::string& lastParsedFile) = 0;
};

