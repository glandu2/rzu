#include "AuctionWriter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zlib.h"
#include <time.h>
#include "Core/PrintfFormats.h"
#include "Core/Utils.h"
#include <algorithm>
#include "Packet/MessageBuffer.h"

static int compressGzip(std::vector<uint8_t>& compressedData, const Bytef *source, uLong sourceLen, int level);

AuctionWriter::AuctionWriter(size_t categoryCount) : firstAuctions(true), diffMode(false), fileNumber(0)
{
	categoryTime.resize(categoryCount, CategoryTime());
}

void AuctionWriter::setDiffInputMode(bool diffMode)
{
	this->diffMode = diffMode;
	if(diffMode) {
		auto it = auctionsState.begin();
		for(; it != auctionsState.end(); ++it) {
			AuctionInfo& auctionInfo = it->second;
			if(auctionInfo.flag != AuctionInfo::AIF_NotProcessed) {
				log(LL_Error, "Switching to diff mode but auction %d is not in AIF_NotProcessed state, setting anyway\n", auctionInfo.uid);
			}
			auctionInfo.flag = AuctionInfo::AIF_Unmodifed;
		}
	}
}

void AuctionWriter::addAuctionInfo(uint32_t uid, uint64_t time, uint16_t category, const uint8_t* data, int len)
{
	if(diffMode) {
		log(LL_Error, "Called addAuctionInfo while in diff mode ! uid: 0x%08X\n", uid);
	}

	auto it = auctionsState.find(uid);
	if(it != auctionsState.end()) {
		AuctionInfo& auctionInfo = it->second;

		if(auctionInfo.flag == AuctionInfo::AIF_NotProcessed)
			auctionInfo.updateTime(time);

		if(auctionInfo.data.size() != (size_t)len ||
		   memcmp(auctionInfo.data.data(), data, len) != 0)
		{
			if(auctionInfo.flag != AuctionInfo::AIF_Added)
				auctionInfo.flag = AuctionInfo::AIF_Updated;

			auctionInfo.category = category;
			auctionInfo.data = std::vector<uint8_t>(data, data + len);
			log(LL_Debug, "Auction info modified: 0x%08X\n", uid);
		} else if(auctionInfo.flag == AuctionInfo::AIF_NotProcessed) {
			auctionInfo.flag = AuctionInfo::AIF_Unmodifed;
		}
	} else {
		auctionsState.insert(std::make_pair(uid,
											AuctionInfo(uid,
														AuctionInfo::AIF_Added,
														time,
														getEstimatedPreviousCategoryBeginTime(category),
														category,
														data, len)));
		log(LL_Debug, "Auction info added: %d\n", uid);
	}

	adjustCategoryTimeRange(category, time);
}

void AuctionWriter::addAuctionInfoDiff(uint32_t uid, uint64_t time, uint64_t previousTime, DiffType diffType, uint16_t category, const uint8_t *data, int len)
{
	if(!diffMode) {
		log(LL_Error, "Called addAuctionInfo while not in diff mode ! uid: 0x%08X\n", uid);
	}

	if(category >= categoryTime.size()) {
		log(LL_Error, "Auction diff with invalid category: uid: 0x%08X, diffType: %d, category: %d, category number: %d\n",
			uid, diffType, category, categoryTime.size());
		return;
	}

	adjustCategoryTimeRange(category, time);

	switch(diffType) {
		case D_Added: {
			auto it = auctionsState.find(uid);
			if(it != auctionsState.end()) {
				log(LL_Error, "Added auction already exists: 0x%08X\n", uid);
			} else {
				if(previousTime && previousTime != getEstimatedPreviousCategoryBeginTime(category)) {
					log(LL_Info, "Added auction previous time mismatch: category previous begin time: %" PRIu64 ", new auction previous time: %" PRIu64 "\n",
						getEstimatedPreviousCategoryBeginTime(category), previousTime);
				}

				uint64_t previousTimeToUse = previousTime ? previousTime : getEstimatedPreviousCategoryBeginTime(category);
				auto insertIt = auctionsState.insert(std::make_pair(uid, AuctionInfo(uid, AuctionInfo::AIF_Added, time, previousTimeToUse, category, data, len)));
				if(insertIt.second == false) {
					log(LL_Error, "Coulnd't insert added auction: 0x%08X\n", uid);
				}
			}
			break;
		}
		case D_Updated: {
			auto it = auctionsState.find(uid);
			if(it == auctionsState.end()) {
				log(LL_Error, "Updated auction not found: 0x%08X\n", uid);

				uint64_t previousTimeToUse = previousTime ? previousTime : getEstimatedPreviousCategoryBeginTime(category);
				auto insertIt = auctionsState.insert(std::make_pair(uid, AuctionInfo(uid, AuctionInfo::AIF_Added, time, previousTimeToUse, category, data, len)));
				if(insertIt.second == false) {
					log(LL_Error, "Coulnd't insert updated auction: 0x%08X\n", uid);
				}
			} else {
				AuctionInfo& auctionInfo = it->second;
				if(previousTime && auctionInfo.getTime() != previousTime) {
					log(LL_Info, "Updated auction previous time mismatch: auction time: %" PRIu64 ", update previous time: %" PRIu64 "\n",
						it->second.getTime(), previousTime);
				}

				if(previousTime)
					auctionInfo.updateTimes(time, previousTime); //set both times
				else if((time_t)auctionInfo.getTime() > getEstimatedPreviousCategoryBeginTime(category))
					auctionInfo.updateTime(time); // auction time after category begin time, trust it and only set current time
				else
					auctionInfo.updateTimes(time, getEstimatedPreviousCategoryBeginTime(category)); // else don't trust it and set both

				auctionInfo.flag = AuctionInfo::AIF_Updated;
				auctionInfo.category = category;
				auctionInfo.data.assign(data, data + len);
			}
			break;
		}
		case D_Deleted: {
			auto it = auctionsState.find(uid);
			if(it == auctionsState.end()) {
				log(LL_Error, "Deleted auction not found: 0x%08X\n", uid);
			} else {
				AuctionInfo& auctionInfo = it->second;
				if(auctionInfo.flag != AuctionInfo::AIF_Unmodifed) {
					log(LL_Error, "Deleted auction is not in AIF_Unmodifed flag: uid: 0x%08X, flag: %d\n", uid, auctionInfo.flag);
				}

				if(getCategoryTime(category).end == 0) {
					log(LL_Info, "Category %d end time is 0, setting to deletion time %" PRIu64 "\n", category, time);
					getCategoryTime(category).end = time;
				} else if(getCategoryTime(category).end != time) {
					log(LL_Warning, "Deleted auction time is not category end time: uid: 0x%08X, time: %" PRIu64 ", category: %d, category end time: %" PRIu64 "\n",
						uid, time, category, getCategoryTime(category).end);
				}
				if(previousTime && auctionInfo.getTime() != previousTime) {
					log(LL_Info, "Deleted auction previousTime is not last auction time: uid: 0x%08X, previousTime: %" PRIu64 ", last auction time: %" PRIu64 "\n",
						uid, previousTime, auctionInfo.getTime());
				}

				auctionInfo.flag = AuctionInfo::AIF_Deleted;

				if(previousTime)
					auctionInfo.updateTimes(time, previousTime);
				else if((time_t)auctionInfo.getTime() > getEstimatedPreviousCategoryBeginTime(category))
					auctionInfo.updateTime(time);
				else
					auctionInfo.updateTimes(time, getEstimatedPreviousCategoryBeginTime(category));
			}
			break;
		}
		case D_Unmodified: {
			auto it = auctionsState.find(uid);
			if(it == auctionsState.end()) {
				fprintf(stderr, "Error: unmodified auction not found: 0x%08X\n", uid);
			} else {
				AuctionInfo& auctionInfo = it->second;
				auctionInfo.flag = AuctionInfo::AIF_Unmodifed;
				if(previousTime && auctionInfo.getTime() != previousTime) {
					log(LL_Info, "Unmodified auction previous time mismatch: auction time: %" PRIu64 ", previous time: %" PRIu64 "\n",
						auctionInfo.getTime(), previousTime);
				}

				if(previousTime)
					auctionInfo.updateTimes(time, previousTime);
				else
					auctionInfo.updateTime(time);
			}
			break;
		}
		default:
			fprintf(stderr, "Error: unsupported flag %d for auction 0x%08X\n", diffType, uid);
			break;
	}

}

void AuctionWriter::dumpAuctions(const std::string& auctionDir, const std::string& auctionFile, bool dumpDiff, bool dumpFull)
{
	time_t dumpTimeStamp = getLastEndCategoryTime();
	if(dumpTimeStamp == 0) {
		log(LL_Warning, "Last category end timestamp is 0, using current timestamp\n");
		dumpTimeStamp = ::time(nullptr);
	}

	processRemovedAuctions(auctionsState);

	if(dumpDiff && !firstAuctions) {
		serializeAuctionInfos(auctionsState, false, fileData);
		writeAuctionDataToFile(auctionDir, auctionFile, fileData, dumpTimeStamp, "_diff");
	}

	if(dumpFull || firstAuctions) {
		serializeAuctionInfos(auctionsState, true, fileData);
		writeAuctionDataToFile(auctionDir, auctionFile, fileData, dumpTimeStamp, "_full");
	}

	postProcessAuctionInfos(auctionsState);
}

bool AuctionWriter::hasAuction(uint32_t uid)
{
	auto it = auctionsState.find(uid);
	if(it == auctionsState.end())
		return false;
	return true;
}

void AuctionWriter::processRemovedAuctions(std::unordered_map<uint32_t, AuctionInfo> &auctionInfos) {
	auto it = auctionInfos.begin();
	for(; it != auctionInfos.end(); ++it) {
		AuctionInfo& auctionInfo = it->second;
		if(auctionInfo.flag == AuctionInfo::AIF_NotProcessed) {
			time_t endCategory = getCategoryTime(auctionInfo.category).end;
			if(endCategory == 0)
				log(LL_Warning, "End of category %d is time 0\n", auctionInfo.category);

			auctionInfo.updateTime(endCategory);
			auctionInfo.flag = AuctionInfo::AIF_Deleted;
			log(LL_Debug, "Auction info removed: 0x%08X\n", auctionInfo.uid);
		}
	}
}

void AuctionWriter::postProcessAuctionInfos(std::unordered_map<uint32_t, AuctionInfo> &auctionInfos) {
	auto it = auctionInfos.begin();
	for(; it != auctionInfos.end();) {
		AuctionInfo& auctionInfo = it->second;
		if(auctionInfo.flag == AuctionInfo::AIF_NotProcessed) {
			log(LL_Error, "Post process auction: found auction in AIF_NotProcessed state, should have been set to AIF_Deleted, uid: 0x%08X\n", auctionInfo.uid);
			++it;
		} else if(auctionInfo.flag == AuctionInfo::AIF_Deleted) {
			it = auctionInfos.erase(it);
		} else {
			auctionInfo.flag = AuctionInfo::AIF_NotProcessed;
			++it;
		}
	}

	for(size_t i = 0; i < categoryTime.size(); i++)
		categoryTime[i].resetTimes();

	firstAuctions = false;
}

template<class Container>
void AuctionWriter::serializeAuctionInfos(const Container &auctionInfos, bool doFullDump, std::vector<uint8_t> &output)
{
	output.clear();

	AUCTION_FILE auctionFile;
	strcpy(auctionFile.signature, "RAH");
	auctionFile.file_version = 3;
	auctionFile.dumpType = doFullDump ? DT_Full : DT_Diff;

	auctionFile.categories.reserve(categoryTime.size());
	for(size_t i = 0; i < categoryTime.size(); i++) {
		AUCTION_CATEGORY_INFO categoryInfo;
		categoryInfo.beginTime = categoryTime[i].begin;
		categoryInfo.endTime = categoryTime[i].end;
		auctionFile.categories.push_back(categoryInfo);
	}

	auctionFile.auctions.reserve(auctionInfos.size());

	auto it = auctionInfos.begin();
	for(; it != auctionInfos.end(); ++it) {
		const AuctionInfo& auctionInfo = getAuctionInfoFromValue(*it);
		int diffType = doFullDump ? AuctionInfo::AIF_Base : getAuctionDiffType(auctionInfo.flag);

		if(diffType >= D_Invalid)
			logStatic(LL_Error, getStaticClassName(), "Invalid diff flag: %d for auction 0x%08X\n", auctionInfo.flag, auctionInfo.uid);

		if(diffType == D_Unmodified && !doFullDump)
			continue;

		AUCTION_INFO auctionItem;
		auctionItem.uid = auctionInfo.uid;
		auctionItem.category = (uint16_t)auctionInfo.category;
		auctionItem.diffType = diffType;
		auctionItem.time = auctionInfo.getTime();
		auctionItem.previousTime = auctionInfo.getPreviousTime();
		auctionItem.data = auctionInfo.data;

		auctionFile.auctions.push_back(auctionItem);
	}

	MessageBuffer buffer(auctionFile.getSize(auctionFile.file_version), auctionFile.file_version);
	auctionFile.serialize(&buffer);
	if(buffer.checkFinalSize() == false) {
		log(LL_Error, "Wrong buffer size, size: %d, field: %s\n", buffer.getSize(), buffer.getFieldInOverflow().c_str());
	} else {
		output.insert(output.end(), buffer.getData(), buffer.getData() + buffer.getSize());
	}
}

void AuctionWriter::writeAuctionDataToFile(std::string auctionsDir, std::string auctionsFile, const std::vector<uint8_t> &data, time_t fileTimeStamp, const char* suffix)
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

	if(data.empty()) {
		log(LL_Warning, "no auction to write to output file %s, skipping write\n", auctionsFilename.c_str());
		return;
	}

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
		int result = compressGzip(compressedData, &data[0], (uLong)data.size(), Z_BEST_COMPRESSION);
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

int AuctionWriter::getAuctionDiffType(AuctionInfo::Flag flag) {
	switch(flag) {
		case AuctionInfo::AIF_Deleted: return D_Deleted;
		case AuctionInfo::AIF_Added: return D_Added;
		case AuctionInfo::AIF_Updated: return D_Updated;
		case AuctionInfo::AIF_Unmodifed: return D_Unmodified;
		case AuctionInfo::AIF_Base: return D_Base;
	}
	return D_Invalid;
}

void AuctionWriter::adjustCategoryTimeRange(size_t category, time_t time)
{
	time_t begin = getCategoryTime(category).begin;
	if(begin == 0 || begin > time)
		getCategoryTime(category).begin = time;

	if(getCategoryTime(category).end < time)
		getCategoryTime(category).end = time;
}

void AuctionWriter::beginCategory(size_t category, time_t time)
{
	time_t lastBeginTime = getCategoryTime(category).begin;
	if(lastBeginTime != 0)
		log(LL_Warning, "Begin category %" PRIuS " has already a begin timestamp: %" PRIdS "\n", category, lastBeginTime);

	if(time == 0)
		log(LL_Warning, "Begin category %" PRIuS " with a 0 timestamp\n", category);

	getCategoryTime(category).begin = time;
	log(LL_Debug, "Begin category %" PRIuS " at time %" PRIdS "\n", category, time);
}

void AuctionWriter::endCategory(size_t category, time_t time)
{
	time_t lastBeginTime = getCategoryTime(category).begin;
	if(lastBeginTime == 0)
		log(LL_Warning, "End category %" PRIuS " but no begin timestamp\n", category);

	if(time == 0)
		log(LL_Warning, "End category %" PRIuS " with a 0 timestamp\n", category);

	getCategoryTime(category).end = time;

	log(LL_Debug, "End category %" PRIuS " at time %" PRIdS "\n", category, time);
}

AuctionWriter::CategoryTime& AuctionWriter::getCategoryTime(size_t category)
{
	if(categoryTime.size() <= category)
		categoryTime.resize(category+1, CategoryTime());

	return categoryTime[category];
}

time_t AuctionWriter::getEstimatedPreviousCategoryBeginTime(size_t category)
{
	time_t maxTime = 0;
	for(ssize_t i = category; i >= 0; i--) {
		if(getCategoryTime(i).previousBegin > maxTime)
			maxTime = getCategoryTime(i).previousBegin;
	}

	return maxTime;
}

time_t AuctionWriter::getLastEndCategoryTime()
{
	time_t lastTime = 0;
	for(size_t i = 0; i < categoryTime.size(); i++) {
		if(categoryTime[i].end > lastTime)
			lastTime = categoryTime[i].end;
	}
	return lastTime;
}

static int compressGzip(std::vector<uint8_t>& compressedData, const Bytef *source, uLong sourceLen, int level) {
	z_stream stream;
	int err;

	memset(&stream, 0, sizeof(stream));

	err = deflateInit2(&stream, level, Z_DEFLATED, MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY);
	if (err != Z_OK) return err;

	compressedData.resize(deflateBound(&stream, sourceLen));

	stream.next_in = (z_const Bytef *)source;
	stream.avail_in = (uInt)sourceLen;
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
