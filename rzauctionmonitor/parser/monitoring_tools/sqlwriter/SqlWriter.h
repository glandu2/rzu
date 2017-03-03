#ifndef SQLWRITER_H
#define SQLWRITER_H

#include "Core/Object.h"
#include "IParser.h"
#include "AuctionSQLWriter.h"

class SqlWriter : public IParser, public Object {
	DECLARE_CLASSNAME(SqlWriter, 0)
public:
	SqlWriter();

	bool isFull() { return false; }

	bool parseAuctions(AuctionComplexDiffWriter* auctionWriter);

	void exportState(std::string filename, const std::string& lastParsedFile);
	void importState(std::string filename, std::string& lastParsedFile);

protected:
	void flushDbInputs();

private:
	std::vector<DB_Item::Input> dbInputs;
};

#endif
