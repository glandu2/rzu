#include "AuctionCommonWriter.h"
#include <stdio.h>
#include <string.h>
#include "zlib.h"
#include <time.h>
#include "Core/PrintfFormats.h"
#include "Core/Utils.h"
#include "Packet/MessageBuffer.h"
#include <memory>

AuctionCommonWriter::AuctionCommonWriter(size_t categoryCount) : categoryTimeManager(categoryCount), fileNumber(0)
{
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

	writeAuctionDataToFile(auctionsDir, auctionsFile, data);
}

void AuctionCommonWriter::writeAuctionDataToFile(std::string auctionsDir, std::string auctionsFile, const std::vector<uint8_t> &data)
{
	std::string auctionsFilename = auctionsDir + "/" + auctionsFile;

	Utils::mkdir(auctionsDir.c_str());

	FILE* file = fopen(auctionsFilename.c_str(), "wb");
	if(!file) {
		log(LL_Error, "Cannot open auction file for write %s\n", auctionsFilename.c_str());
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

bool AuctionCommonWriter::readAuctionDataFromFile(std::string auctionsDir, std::string auctionsFile, std::vector<uint8_t>& data)
{
	std::unique_ptr<FILE, int(*)(FILE*)> file(nullptr, &fclose);
	std::string filename = auctionsDir + "/" + auctionsFile;

	data.clear();

	file.reset(fopen(filename.c_str(), "rb"));
	if(!file) {
		log(LL_Error, "Cannot open auction file for read %s\n", filename.c_str());
		return false;
	}

	fseek(file.get(), 0, SEEK_END);
	size_t fileSize = ftell(file.get());
	fseek(file.get(), 0, SEEK_SET);

	if(fileSize > 50*1024*1024) {
		log(LL_Error, "State file %s size too large (over 50MB): %d\n", filename.c_str(), fileSize);
		return false;
	}

	size_t pos = auctionsFile.find_last_of(".gz");
	if(pos == (auctionsFile.size() - 1)) {
		log(LL_Info, "Reading compressed data from file %s\n", auctionsFile.c_str());

		std::vector<uint8_t> compressedData;
		compressedData.resize(fileSize);
		size_t readDataSize = fread(compressedData.data(), 1, fileSize, file.get());
		if(readDataSize != fileSize) {
			log(LL_Error, "Coulnd't read all file data: %s, size: %ld, read: %ld\n", filename.c_str(), (long int)fileSize, (long int)readDataSize);
			return false;
		}

		int result = uncompressGzip(data, compressedData);
		if(result != Z_OK) {
			log(LL_Error, "Failed to uncompress %s: %d\n", filename.c_str(), result);
			data.clear();
			return false;
		}
	} else {
		log(LL_Info, "Reading data from file %s\n", auctionsFile.c_str());
		data.resize(fileSize);
		size_t readDataSize = fread(data.data(), 1, fileSize, file.get());
		if(readDataSize != fileSize) {
			log(LL_Error, "Coulnd't read all file data: %s, size: %ld, read: %ld\n", filename.c_str(), (long int)fileSize, (long int)readDataSize);
			return false;
		}
	}

	return true;
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

int AuctionCommonWriter::uncompressGzip(std::vector<uint8_t>& uncompressedData, const std::vector<uint8_t>& compressedData) {
	z_stream stream;
	int err;
	size_t outputIndex = 0;

	memset(&stream, 0, sizeof(stream));

	err = inflateInit2(&stream, 16+MAX_WBITS);
	if (err != Z_OK) return err;

	stream.avail_in = compressedData.size();
	stream.next_in = compressedData.data();

	/* run inflate() on input until output buffer not full */
	do {
		uncompressedData.resize(uncompressedData.size() + 1024*1024);
		stream.avail_out = uncompressedData.size() - outputIndex;
		stream.next_out = &uncompressedData[outputIndex];
		err = inflate(&stream, Z_NO_FLUSH);
		switch (err) {
			case Z_NEED_DICT:
				err = Z_DATA_ERROR;     /* and fall through */
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
			case Z_STREAM_ERROR:
				(void)inflateEnd(&stream);
				return err;
		}
		outputIndex = uncompressedData.size() - stream.avail_out;
	} while (stream.avail_in > 0);

	err = inflateEnd(&stream);

	uncompressedData.resize(stream.total_out);

	return err;
}
