#pragma once

#include "AuctionFile.h"
#include "Core/Object.h"
#include "Extern.h"
#include <stdint.h>
#include <vector>

class RZAUCTION_EXTERN CategoryTimeManager : public Object {
	DECLARE_CLASSNAME(CategoryTimeManager, 0)

public:
	CategoryTimeManager(size_t categoryCount);

private:
	struct CategoryTime {
		time_t previousBegin;
		time_t previousEnd;
		time_t begin;
		time_t end;

		CategoryTime() : previousBegin(0), previousEnd(0), begin(0), end(0) {}
		void resetTimes(time_t minimalBegin) {
			previousBegin = begin;
			previousEnd = end;
			begin = end = minimalBegin;
		}
	};

public:
	void beginCategory(size_t category, time_t time);
	void endCategory(size_t category, time_t time);

	void adjustCategoryTimeRange(size_t category, time_t time);
	void resetCategoryTime();

	time_t getEstimatedPreviousCategoryBeginTime(size_t category);
	time_t getEstimatedCategoryBeginTime(size_t category);
	time_t getEstimatedCategoryEndTime(size_t category);
	time_t getLastEndCategoryTime();
	size_t getCategoryNumber() { return categoryTime.size(); }

	void serializeHeader(AUCTION_HEADER& header, DumpType dumpType);
	void deserializeHeader(const AUCTION_HEADER& header);

private:
	CategoryTime& getCategoryTime(size_t category);

private:
	std::vector<CategoryTime> categoryTime;
};

