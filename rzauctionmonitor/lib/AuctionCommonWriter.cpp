#include "AuctionCommonWriter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zlib.h"
#include <time.h>
#include "Core/PrintfFormats.h"
#include "Core/Utils.h"
#include <algorithm>
#include "Packet/MessageBuffer.h"

AuctionCommonWriter::AuctionCommonWriter(size_t categoryCount) : fileNumber(0)
{
	categoryTime.resize(categoryCount, CategoryTime());
}

void AuctionCommonWriter::beginProcess()
{
	resetAuctionProcess();
}

void AuctionCommonWriter::endProcess()
{
	processRemainingAuctions();
}

void AuctionCommonWriter::dumpAuctions(const std::string& auctionDir, const std::string& auctionFile, bool dumpDiff, bool dumpFull)
{
	time_t dumpTimeStamp = getLastEndCategoryTime();
	if(dumpTimeStamp == 0) {
		log(LL_Warning, "Last category end timestamp is 0, using current timestamp\n");
		dumpTimeStamp = ::time(nullptr);
	}

	if(dumpDiff) {
		serializeAuctionInfos(false, fileData);
		writeAuctionDataToFile(auctionDir, auctionFile, fileData, dumpTimeStamp, "_diff");
	}

	if(dumpFull) {
		serializeAuctionInfos(true, fileData);
		writeAuctionDataToFile(auctionDir, auctionFile, fileData, dumpTimeStamp, "_full");
	}
}

void AuctionCommonWriter::dumpAuctions(std::vector<uint8_t> &auctionData, bool doFulldump)
{
	serializeAuctionInfos(doFulldump, auctionData);
}

void AuctionCommonWriter::resetCategoryTime() {
	for(size_t i = 0; i < categoryTime.size(); i++)
		categoryTime[i].resetTimes();
}

void AuctionCommonWriter::writeAuctionDataToFile(std::string auctionsDir, std::string auctionsFile, const std::vector<uint8_t> &data, time_t fileTimeStamp, const char* suffix)
{
	char filenameSuffix[256];
	struct tm localtm;

	Utils::getGmTime(fileTimeStamp, &localtm);

	sprintf(filenameSuffix, "_%04d%02d%02d_%02d%02d%02d_%04X%s",
	        localtm.tm_year, localtm.tm_mon, localtm.tm_mday,
	        localtm.tm_hour, localtm.tm_min, localtm.tm_sec, fileNumber,
	        suffix ? suffix : "");
	fileNumber = (fileNumber + 1) & 0xFFFF;

	auctionsFile.insert(auctionsFile.find_first_of('.'), filenameSuffix);

	std::string auctionsFilename = auctionsDir + "/" + auctionsFile;

	Utils::mkdir(auctionsDir.c_str());

	FILE* file = fopen(auctionsFilename.c_str(), "wb");
	if(!file) {
		log(LL_Error, "Cannot open auction file %s\n", auctionsFilename.c_str());
		return;
	}

	size_t pos = auctionsFile.find_last_of(".gz");
	if(pos == (auctionsFile.size() - 1)) {
		log(LL_Info, "Writting compressed data to file %s\n", auctionsFile.c_str());

		std::vector<uint8_t> compressedData;
		int result = compressGzip(compressedData, data, Z_BEST_COMPRESSION);
		if(result == Z_OK) {
			if(fwrite(&compressedData[0], sizeof(uint8_t), compressedData.size(), file) != compressedData.size()) {
				log(LL_Error, "Failed to write data to file %s: error %d\n", auctionsFilename.c_str(), errno);
			}
		} else {
			log(LL_Error, "Failed to compress %d bytes: %d\n", (int)data.size(), result);
		}
	} else {
		log(LL_Info, "Writting data to file %s\n", auctionsFile.c_str());
		if(fwrite(&data[0], sizeof(uint8_t), data.size(), file) != data.size()) {
			log(LL_Error, "Failed to write data to file %s: error %d\n", auctionsFilename.c_str(), errno);
		}
	}

	fclose(file);
}

void AuctionCommonWriter::serializeHeader(AUCTION_HEADER &header, DumpType dumpType)
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

void AuctionCommonWriter::deserializeHeader(AUCTION_HEADER &header)
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

void AuctionCommonWriter::adjustCategoryTimeRange(size_t category, time_t time)
{
	time_t begin = getCategoryTime(category).begin;
	if(begin == 0 || begin > time)
		getCategoryTime(category).begin = time;

	if(getCategoryTime(category).end < time)
		getCategoryTime(category).end = time;
}

void AuctionCommonWriter::beginCategory(size_t category, time_t time)
{
	time_t lastBeginTime = getCategoryTime(category).begin;
	if(lastBeginTime != 0)
		log(LL_Warning, "Begin category %" PRIuS " has already a begin timestamp: %" PRIdS "\n", category, lastBeginTime);

	if(time == 0)
		log(LL_Warning, "Begin category %" PRIuS " with a 0 timestamp\n", category);

	getCategoryTime(category).begin = time;
	log(LL_Debug, "Begin category %" PRIuS " at time %" PRIdS "\n", category, time);
}

void AuctionCommonWriter::endCategory(size_t category, time_t time)
{
	time_t lastBeginTime = getCategoryTime(category).begin;
	if(lastBeginTime == 0)
		log(LL_Warning, "End category %" PRIuS " but no begin timestamp\n", category);

	if(time == 0)
		log(LL_Warning, "End category %" PRIuS " with a 0 timestamp\n", category);

	getCategoryTime(category).end = time;

	log(LL_Debug, "End category %" PRIuS " at time %" PRIdS "\n", category, time);
}

AuctionCommonWriter::CategoryTime& AuctionCommonWriter::getCategoryTime(size_t category)
{
	if(categoryTime.size() <= category)
		categoryTime.resize(category+1, CategoryTime());

	return categoryTime[category];
}

time_t AuctionCommonWriter::getEstimatedPreviousCategoryBeginTime(size_t category)
{
	time_t maxTime = 0;
	//get maximum previous time of all categories preceding "category"
	for(ssize_t i = category; i >= 0; i--) {
		if(getCategoryTime(i).previousBegin > maxTime)
			maxTime = getCategoryTime(i).previousBegin;
	}

	return maxTime;
}

time_t AuctionCommonWriter::getEstimatedCategoryBeginTime(size_t category)
{
	time_t maxTime = 0;
	//get maximum previous time of all categories preceding "category"
	for(ssize_t i = category; i >= 0; i--) {
		if(getCategoryTime(i).begin > maxTime)
			maxTime = getCategoryTime(i).begin;
	}

	return maxTime;
}

time_t AuctionCommonWriter::getLastEndCategoryTime()
{
	time_t lastTime = 0;
	for(size_t i = 0; i < categoryTime.size(); i++) {
		if(categoryTime[i].end > lastTime)
			lastTime = categoryTime[i].end;
	}
	return lastTime;
}

int AuctionCommonWriter::compressGzip(std::vector<uint8_t>& compressedData, const std::vector<uint8_t>& sourceData, int level) {
	z_stream stream;
	int err;

	memset(&stream, 0, sizeof(stream));

	err = deflateInit2(&stream, level, Z_DEFLATED, MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY);
	if (err != Z_OK) return err;

	compressedData.resize(deflateBound(&stream, sourceData.size()));

	stream.next_in = sourceData.data();
	stream.avail_in = (uInt)sourceData.size();
	stream.next_out = compressedData.data();
	stream.avail_out = (uInt)compressedData.size();
	if ((uLong)stream.avail_out != compressedData.size())
		return Z_BUF_ERROR;

	err = deflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END) {
		deflateEnd(&stream);
		return err == Z_OK ? Z_BUF_ERROR : err;
	}

	compressedData.resize(stream.total_out);

	err = deflateEnd(&stream);
	return err;
}
