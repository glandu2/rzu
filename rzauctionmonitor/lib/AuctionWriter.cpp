#include "AuctionWriter.h"
#include "Core/PrintfFormats.h"
#include "Core/ScopeGuard.h"
#include "Core/Utils.h"
#include "Packet/MessageBuffer.h"
#include "zlib.h"
#include <memory>
#include <stdio.h>
#include <string.h>
#include <time.h>

#pragma pack(push, 1)
struct AuctionHeaderV0 {
	int32_t size;
	int64_t time;
	int32_t category;
	int32_t page;

	int getSize() const { return size; }
	int16_t getVersion() const { return 0; }
	int16_t getFlag() const { return D_Base; }
	int64_t getTime() const { return time; }
	int64_t getPreviousTime() const { return 0; }
	int16_t getCategory() const { return category; }
};

struct AuctionHeaderV1 {
	int32_t size;
	int16_t version;
	int16_t flag;
	int64_t time;
	int16_t category;

	int getSize() const { return size - sizeof(*this); }
	int16_t getVersion() const { return version; }
	int16_t getFlag() const { return flag; }
	int64_t getTime() const { return time; }
	int64_t getPreviousTime() const { return 0; }
	int16_t getCategory() const { return category; }
};

struct AuctionHeaderV2 {
	int32_t size;
	int16_t version;
	int16_t flag;
	int64_t time;
	int64_t previousTime;
	int16_t category;

	int getSize() const { return size - sizeof(*this); }
	int16_t getVersion() const { return version; }
	int16_t getFlag() const { return flag; }
	int64_t getTime() const { return time; }
	int64_t getPreviousTime() const { return previousTime; }
	int16_t getCategory() const { return category; }
};
#pragma pack(pop)

void AuctionWriter::writeAuctionDataToFile(std::string auctionsFile, const std::vector<uint8_t>& data) {
	std::unique_ptr<FILE, int (*)(FILE*)> file(nullptr, &fclose);

	file.reset(fopen(auctionsFile.c_str(), "wb"));
	if(!file) {
		logStatic(LL_Error, getStaticClassName(), "Cannot open auction file for write %s\n", auctionsFile.c_str());
		return;
	}

	size_t pos = auctionsFile.find_last_of(".gz");
	if(pos == (auctionsFile.size() - 1)) {
		logStatic(LL_Debug, getStaticClassName(), "Writing compressed data to file %s\n", auctionsFile.c_str());

		std::vector<uint8_t> compressedData;
		const char* zlibMessage;
		int result = compressGzip(compressedData, data, Z_BEST_COMPRESSION, zlibMessage);
		if(result == Z_OK) {
			if(fwrite(&compressedData[0], sizeof(uint8_t), compressedData.size(), file.get()) !=
			   compressedData.size()) {
				logStatic(LL_Error,
				          getStaticClassName(),
				          "Failed to write data to file %s: error %d\n",
				          auctionsFile.c_str(),
				          errno);
			}
		} else {
			logStatic(LL_Error,
			          getStaticClassName(),
			          "Failed to compress %d bytes: %d: %s\n",
			          (int) data.size(),
			          result,
			          zlibMessage ? zlibMessage : "");
		}
	} else {
		logStatic(LL_Info, getStaticClassName(), "Writing data to file %s\n", auctionsFile.c_str());
		if(fwrite(&data[0], sizeof(uint8_t), data.size(), file.get()) != data.size()) {
			logStatic(LL_Error,
			          getStaticClassName(),
			          "Failed to write data to file %s: error %d\n",
			          auctionsFile.c_str(),
			          errno);
		}
	}
}

bool AuctionWriter::readAuctionDataFromFile(std::string auctionsFile,
                                            std::vector<AuctionWriter::file_data_byte>& data) {
	std::unique_ptr<FILE, int (*)(FILE*)> file(nullptr, &fclose);

	data.clear();

	file.reset(fopen(auctionsFile.c_str(), "rb"));
	if(!file) {
		logStatic(LL_Error, getStaticClassName(), "Cannot open auction file for read %s\n", auctionsFile.c_str());
		return false;
	}

	fseek(file.get(), 0, SEEK_END);
	size_t fileSize = ftell(file.get());
	fseek(file.get(), 0, SEEK_SET);

	if(fileSize == 0) {
		// Empty file
		return true;
	}

	if(fileSize > 50 * 1024 * 1024) {
		logStatic(LL_Error,
		          getStaticClassName(),
		          "State file %s size too large (over 50MB): %d\n",
		          auctionsFile.c_str(),
		          (int) fileSize);
		return false;
	}

	size_t pos = auctionsFile.find_last_of(".gz");
	if(pos == (auctionsFile.size() - 1)) {
		logStatic(LL_Debug, getStaticClassName(), "Reading compressed data from file %s\n", auctionsFile.c_str());

		std::vector<uint8_t> compressedData;
		compressedData.resize(fileSize);
		size_t readDataSize = fread(compressedData.data(), 1, fileSize, file.get());
		if(readDataSize != fileSize) {
			logStatic(LL_Error,
			          getStaticClassName(),
			          "Coulnd't read all file data: %s, size: %ld, read: %ld\n",
			          auctionsFile.c_str(),
			          (long int) fileSize,
			          (long int) readDataSize);
			return false;
		}

		const char* zlibMessage;
		int result = uncompressGzip(data, compressedData, zlibMessage);
		if(result != Z_OK) {
			logStatic(LL_Error,
			          getStaticClassName(),
			          "Failed to uncompress %s: %d: %s\n",
			          auctionsFile.c_str(),
			          result,
			          zlibMessage ? zlibMessage : "");
			data.clear();
			return false;
		}
	} else {
		logStatic(LL_Info, getStaticClassName(), "Reading data from file %s\n", auctionsFile.c_str());
		data.resize(fileSize);
		size_t readDataSize = fread(data.data(), 1, fileSize, file.get());
		if(readDataSize != fileSize) {
			logStatic(LL_Error,
			          getStaticClassName(),
			          "Coulnd't read all file data: %s, size: %ld, read: %ld\n",
			          auctionsFile.c_str(),
			          (long int) fileSize,
			          (long int) readDataSize);
			return false;
		}
	}

	return true;
}

bool AuctionWriter::getAuctionFileFormat(const std::vector<AuctionWriter::file_data_byte>& data,
                                         int* version,
                                         AuctionFileFormat* format) {
	union FileHeader {
		struct OldHeader {
			uint32_t size;
			uint16_t version;
			uint16_t padding;
		} oldHeader;
		AuctionFileHeader newHeader;
	};
	const FileHeader* header = reinterpret_cast<const FileHeader*>(data.data());

	if(data.size() > sizeof(header->newHeader) && !strcmp(header->newHeader.signature, "RAH")) {
		*format = AFF_Complex;
		*version = header->newHeader.file_version;
	} else if(data.size() > sizeof(header->newHeader) && !strcmp(header->newHeader.signature, "RHS")) {
		*format = AFF_Simple;
		*version = header->newHeader.file_version;
	} else {
		int trueVersion = 0;

		if(data.size() > sizeof(header->oldHeader)) {
			trueVersion = header->oldHeader.version;

			if(trueVersion > 1000)
				trueVersion = 0;

			if(header->oldHeader.size > 4096 || trueVersion > 2) {
				logStatic(LL_Error,
				          getStaticClassName(),
				          "Bad old format, size: %d, version: %d (limit: 4096, 2)\n",
				          header->oldHeader.size,
				          trueVersion);
				return false;
			}
		}

		*format = AFF_Old;
		*version = trueVersion;
	}

	return true;
}

template<class T> bool AuctionWriter::deserializeFile(const std::vector<file_data_byte>& buffer, T* auctionFile) {
	if(buffer.size() < sizeof(AuctionFileHeader)) {
		logStatic(LL_Error, getStaticClassName(), "File size too small, can't deserialize\n");
		return false;
	}

	uint32_t version = ((AuctionFileHeader*) buffer.data())->file_version;

	MessageBuffer structBuffer(buffer.data(), buffer.size(), version);

	auctionFile->deserialize(&structBuffer);
	if(!structBuffer.checkFinalSize()) {
		logStatic(LL_Error, getStaticClassName(), "Invalid file data, can't deserialize\n");
		return false;
	}

	return true;
}

template<class AuctionHeader>
bool AuctionWriter::deserializeOldFile(const std::vector<file_data_byte>& data, AUCTION_SIMPLE_FILE* auctionFile) {
	const file_data_byte* p = data.data();
	const file_data_byte* const endp = data.data() + data.size();
	bool reserved = false;

	while(p < endp) {
		const AuctionHeader* header = reinterpret_cast<const AuctionHeader*>(p);
		const size_t remainingDataSize = endp - p;

		if(sizeof(*header) > remainingDataSize || (sizeof(*header) + header->getSize()) > remainingDataSize) {
			logStatic(LL_Error,
			          getStaticClassName(),
			          "Data size too large: %d at offset %d\n",
			          (int) sizeof(*header),
			          (int) (p - data.data()));
			return false;
		} else if((sizeof(*header) + header->getSize()) > remainingDataSize) {
			logStatic(LL_Error,
			          getStaticClassName(),
			          "Data size too large: %d at offset %d\n",
			          (int) sizeof(*header) + header->getSize(),
			          (int) (p - data.data()));
			return false;
		}

		if(!reserved) {
			reserved = true;
			auctionFile->auctions.reserve(data.size() / (sizeof(*header) + header->getSize()));
		}

		const uint8_t* rawData = reinterpret_cast<const uint8_t*>(p + sizeof(*header));
		size_t dataSize = header->getSize();
		const uint32_t* uid = reinterpret_cast<const uint32_t*>(rawData);

		auctionFile->auctions.push_back(AUCTION_SIMPLE_INFO());
		AUCTION_SIMPLE_INFO& auctionInfo = auctionFile->auctions.back();

		auctionInfo.uid = *uid;
		auctionInfo.time = header->getTime();
		auctionInfo.previousTime = header->getPreviousTime();
		auctionInfo.diffType = header->getFlag();
		auctionInfo.category = header->getCategory();
		auctionInfo.epic = 0xFFFFFF;
		auctionInfo.data = std::vector<uint8_t>(rawData, rawData + dataSize);

		p += sizeof(*header) + header->getSize();
	}

	return true;
}

bool AuctionWriter::deserialize(AUCTION_SIMPLE_FILE* file, const std::vector<file_data_byte>& data) {
	int version;
	AuctionFileFormat fileFormat;

	if(!getAuctionFileFormat(data, &version, &fileFormat)) {
		logStatic(LL_Error, getStaticClassName(), "Failed to get file format\n");
		return false;
	}

	switch(fileFormat) {
		case AFF_Simple:
			if(!deserializeFile(data, file))
				return false;
			break;

		case AFF_Complex: {
			AUCTION_FILE complexFile;

			if(!deserializeFile(data, &complexFile))
				return false;

			file->header = complexFile.header;
			file->auctions.clear();
			file->auctions.reserve(complexFile.auctions.size());
			for(size_t i = 0; i < complexFile.auctions.size(); i++) {
				const AUCTION_INFO& auctionInfo = complexFile.auctions[i];
				file->auctions.emplace_back();
				AUCTION_SIMPLE_INFO& auctionSimpleInfo = file->auctions.back();

				auctionSimpleInfo.uid = auctionInfo.uid;
				auctionSimpleInfo.time = auctionInfo.time;
				auctionSimpleInfo.previousTime = auctionInfo.previousTime;
				auctionSimpleInfo.diffType = auctionInfo.diffType;
				auctionSimpleInfo.category = auctionInfo.category;
				auctionSimpleInfo.epic = auctionInfo.epic;
				auctionSimpleInfo.data = auctionInfo.data;
			}
			break;
		}

		case AFF_Old:
			memcpy(file->header.signature, "RHS", 4);
			file->header.file_version = AUCTION_LATEST;
			file->header.dumpType = DT_UnknownDumpType;
			switch(version) {
				case 0:
					if(!deserializeOldFile<AuctionHeaderV0>(data, file))
						return false;
					break;

				case 1:
					if(!deserializeOldFile<AuctionHeaderV1>(data, file))
						return false;
					break;

				case 2:
					if(!deserializeOldFile<AuctionHeaderV2>(data, file))
						return false;
					break;

				default:
					logStatic(LL_Error, getStaticClassName(), "Invalid version %d for old file format\n", version);
					return false;
			}
			break;
	}

	return true;
}

bool AuctionWriter::deserialize(AUCTION_FILE* file, const std::vector<file_data_byte>& data) {
	return deserializeFile(data, file);
}

int AuctionWriter::compressGzip(std::vector<uint8_t>& compressedData,
                                const std::vector<uint8_t>& sourceData,
                                int level,
                                const char*& msg) {
	z_stream stream;
	int err;
	msg = nullptr;

	memset(&stream, 0, sizeof(stream));

	guard_on_exit(setMsg, [&] { msg = stream.msg; });

	err = deflateInit2(&stream, level, Z_DEFLATED, MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY);
	if(err != Z_OK) {
		return err;
	}

	compressedData.resize(deflateBound(&stream, static_cast<uLong>(sourceData.size())));

	stream.next_in = sourceData.data();
	stream.avail_in = (uInt) sourceData.size();
	stream.next_out = compressedData.data();
	stream.avail_out = (uInt) compressedData.size();
	if((uLong) stream.avail_out != compressedData.size())
		return Z_BUF_ERROR;

	err = deflate(&stream, Z_FINISH);
	if(err != Z_STREAM_END) {
		deflateEnd(&stream);
		return err == Z_OK ? Z_BUF_ERROR : err;
	}

	compressedData.resize(stream.total_out);

	err = deflateEnd(&stream);
	return err;
}

int AuctionWriter::uncompressGzip(std::vector<AuctionWriter::file_data_byte>& uncompressedData,
                                  const std::vector<uint8_t>& compressedData,
                                  const char*& msg) {
	z_stream stream;
	int err;
	size_t outputIndex = 0;

	memset(&stream, 0, sizeof(stream));

	guard_on_exit(setMsg, [&] { msg = stream.msg; });

	err = inflateInit2(&stream, 16 + MAX_WBITS);
	if(err != Z_OK)
		return err;

	stream.avail_in = static_cast<uLong>(compressedData.size());
	stream.next_in = compressedData.data();

	uint32_t dataSize = 6 * 1024 * 1024;

	if(compressedData.size() < 4) {
		logStatic(LL_Warning,
		          "AuctionWriter",
		          "Can't read uncompressed size from file, file too short: %u\n",
		          (unsigned int) compressedData.size());
	} else {
		uint32_t header = static_cast<uint32_t>(compressedData[0]) | static_cast<uint32_t>(compressedData[0]) << 8;
		uint32_t dataSizeFromFile = static_cast<uint32_t>(compressedData[compressedData.size() - 4]) |
		                            static_cast<uint32_t>(compressedData[compressedData.size() - 3]) << 8 |
		                            static_cast<uint32_t>(compressedData[compressedData.size() - 2]) << 16 |
		                            static_cast<uint32_t>(compressedData[compressedData.size() - 1]) << 24;

		if(header != 0x8B1F) {
			// File is not GZIPed
		} else if(dataSizeFromFile > 10 * 1024 * 1024) {
			logStatic(
			    LL_Warning,
			    "AuctionWriter",
			    "Uncompressed size read from file is too large (> 10MB): %u, decompressing using default buffer size\n",
			    (unsigned int) dataSizeFromFile);
		} else if(dataSizeFromFile == 0) {
			logStatic(LL_Warning, "AuctionWriter", "Uncompressed size from file is 0\n");
		} else {
			dataSize = dataSizeFromFile;
		}
	}

	/* run inflate() on input until output buffer not full */
	do {
		if(uncompressedData.empty() && dataSize > 0) {
			uncompressedData.resize(dataSize);
		} else {
			if(!uncompressedData.empty())
				logStatic(LL_Warning,
				          "AuctionWriter",
				          "Size of %u wasn't enough for uncompressed zlib data\n",
				          (unsigned int) uncompressedData.size());
			else
				uncompressedData.resize(uncompressedData.size() + 7 * 1024 * 1024);
		}
		stream.avail_out = static_cast<uLong>(uncompressedData.size()) - outputIndex;
		stream.next_out = reinterpret_cast<Bytef*>(&uncompressedData[outputIndex]);
		err = inflate(&stream, Z_NO_FLUSH);
		switch(err) {
			case Z_NEED_DICT:
				err = Z_DATA_ERROR; /* and fall through */
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
			case Z_STREAM_ERROR:
				(void) inflateEnd(&stream);
				return err;
		}
		outputIndex = uncompressedData.size() - stream.avail_out;
	} while(stream.avail_in > 0);

	err = inflateEnd(&stream);

	uncompressedData.resize(stream.total_out);

	return err;
}
