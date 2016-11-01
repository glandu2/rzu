#include "CategoryTimeManager.h"
#include <stdio.h>
#include <string.h>
#include "zlib.h"
#include <time.h>
#include "Core/PrintfFormats.h"
#include "Core/Utils.h"
#include "Packet/MessageBuffer.h"

CategoryTimeManager::CategoryTimeManager(size_t categoryCount)
{
	categoryTime.resize(categoryCount, CategoryTime());
}

void CategoryTimeManager::beginCategory(size_t category, time_t time)
{
	time_t lastBeginTime = getCategoryTime(category).begin;
	if(lastBeginTime != 0)
		log(LL_Warning, "Begin category %" PRIuS " has already a begin timestamp: %" PRId64 "\n", category, (int64_t)lastBeginTime);

	if(time == 0)
		log(LL_Warning, "Begin category %" PRIuS " with a 0 timestamp\n", category);

	getCategoryTime(category).begin = time;
	log(LL_Debug, "Begin category %" PRIuS " at time %" PRId64 "\n", category, (int64_t)time);
}

void CategoryTimeManager::endCategory(size_t category, time_t time)
{
	time_t lastBeginTime = getCategoryTime(category).begin;
	if(lastBeginTime == 0)
		log(LL_Warning, "End category %" PRIuS " but no begin timestamp\n", category);

	if(time == 0)
		log(LL_Warning, "End category %" PRIuS " with a 0 timestamp\n", category);

	getCategoryTime(category).end = time;

	log(LL_Debug, "End category %" PRIuS " at time %" PRId64 "\n", category, (int64_t)time);
}

void CategoryTimeManager::resetCategoryTime() {
	time_t lastEndCategoryTime = getLastEndCategoryTime();
	for(size_t i = 0; i < categoryTime.size(); i++)
		categoryTime[i].resetTimes(lastEndCategoryTime);
}

void CategoryTimeManager::adjustCategoryTimeRange(size_t category, time_t time)
{
	time_t begin = getCategoryTime(category).begin;
	if(begin == 0 || begin > time)
		getCategoryTime(category).begin = time;

	if(getCategoryTime(category).end < time)
		getCategoryTime(category).end = time;
}

CategoryTimeManager::CategoryTime& CategoryTimeManager::getCategoryTime(size_t category)
{
	if(categoryTime.size() <= category) {
		log(LL_Warning, "Category out of maximum category index: %" PRIuS " > %" PRIuS ", auto-resizings\n", category, categoryTime.size()-1);
		categoryTime.resize(category+1, CategoryTime());
	}

	return categoryTime[category];
}

time_t CategoryTimeManager::getEstimatedPreviousCategoryBeginTime(size_t category)
{
	time_t maxTime = 0;
	//get maximum previous time of all categories preceding "category"
	for(ssize_t i = category; i >= 0; i--) {
		if(getCategoryTime(i).previousBegin > maxTime)
			maxTime = getCategoryTime(i).previousBegin;
	}

	return maxTime;
}

time_t CategoryTimeManager::getEstimatedCategoryBeginTime(size_t category)
{
	time_t maxTime = 0;
	//get maximum previous time of all categories preceding "category"
	for(ssize_t i = category; i >= 0; i--) {
		if(getCategoryTime(i).begin > maxTime)
			maxTime = getCategoryTime(i).begin;
	}

	return maxTime;
}

time_t CategoryTimeManager::getEstimatedCategoryEndTime(size_t category)
{
	time_t maxTime = 0;
	//get maximum previous time of all categories preceding "category"
	for(ssize_t i = category; i >= 0; i--) {
		if(getCategoryTime(i).end > maxTime)
			maxTime = getCategoryTime(i).end;
	}

	return maxTime;
}

time_t CategoryTimeManager::getLastEndCategoryTime()
{
	time_t lastTime = 0;
	for(size_t i = 0; i < categoryTime.size(); i++) {
		if(categoryTime[i].end > lastTime)
			lastTime = categoryTime[i].end;
	}
	return lastTime;
}

void CategoryTimeManager::serializeHeader(AUCTION_HEADER &header, DumpType dumpType)
{
	strcpy(header.signature, "RAH");
	header.file_version = AUCTION_LATEST;
	header.dumpType = dumpType;

	header.categories.reserve(categoryTime.size());
	for(size_t i = 0; i < categoryTime.size(); i++) {
		AUCTION_CATEGORY_INFO categoryInfo;
		categoryInfo.previousBegin = categoryTime[i].previousBegin;
		categoryInfo.beginTime = categoryTime[i].begin;
		categoryInfo.endTime = categoryTime[i].end;
		header.categories.push_back(categoryInfo);
	}
}

void CategoryTimeManager::deserializeHeader(AUCTION_HEADER &header)
{
	categoryTime.clear();
	for(size_t i = 0; i < header.categories.size(); i++) {
		AUCTION_CATEGORY_INFO& categoryInfo = header.categories[i];
		CategoryTime category;

		category.previousBegin = categoryInfo.previousBegin;
		category.begin = categoryInfo.beginTime;
		category.end = categoryInfo.endTime;

		categoryTime.push_back(category);
	}
}
