#include "AuctionFile.h"
#include "AuctionWriter.h"
#include "Config/GlobalCoreConfig.h"
#include "Core/Log.h"
#include "LibRzuInit.h"
#include "Packet/JSONWriter.h"
#include <Packet/MessageBuffer.h>
#include <stdint.h>
#include <vector>

cval<bool>& compactJson = CFG_CREATE("compactjson", false);

// clang-format off
#define PIPELINE_STATE_DEF(_) \
    _(simple)  (uint16_t, file_version) \
    _(count)   (uint8_t, lastParsedFile) \
    _(dynstring)(lastParsedFile, false) \
    _(simple)  (AUCTION_FILE, auctionData)
    CREATE_STRUCT(PIPELINE_STATE);
#undef PIPELINE_STATE_DEF
// clang-format on

template<class AUCTION_FILE> int writeJson(const AUCTION_FILE& auctionFile, packet_version_t version) {
	JSONWriter jsonWriter(version, compactJson.get());
	auctionFile.serialize(&jsonWriter);
	jsonWriter.finalize();
	Object::logStatic(Object::LL_Fatal, "main", "%s\n", jsonWriter.toString().c_str());

	return 0;
}

int main(int argc, char* argv[]) {
	LibRzuScopedUse useLibRzu;

	ConfigInfo::get()->init(argc, argv);

	Log mainLogger(GlobalCoreConfig::get()->log.enable,
	               GlobalCoreConfig::get()->log.level,
	               GlobalCoreConfig::get()->log.consoleLevel,
	               GlobalCoreConfig::get()->log.dir,
	               GlobalCoreConfig::get()->log.file,
	               GlobalCoreConfig::get()->log.maxQueueSize);
	Log::setDefaultLogger(&mainLogger);

	ConfigInfo::get()->dump();

	if(argc < 2) {
		Object::logStatic(Object::LL_Info, "main", "Usage: %s auctions.bin\n", argv[0]);
		return 0;
	}

	int i;
	for(i = 1; i < argc; i++) {
		const char* filename = argv[i];

#ifdef _WIN32
		if(filename[0] == '/')
			continue;
#endif
		if(filename[0] == '-')
			continue;

		std::vector<AuctionWriter::file_data_byte> data;
		int version;
		AuctionFileFormat fileFormat;

		if(!AuctionWriter::readAuctionDataFromFile(filename, data)) {
			Object::logStatic(Object::LL_Error, "main", "Cant read file %s\n", filename);
			return 1;
		}

		if(!AuctionWriter::getAuctionFileFormat(data, &version, &fileFormat)) {
			Object::logStatic(Object::LL_Error, "main", "Invalid file, unrecognized header signature: %s\n", filename);

			PIPELINE_STATE file;

			MessageBuffer structBuffer(data.data(), data.size(), 0xFFFFFF);

			file.deserialize(&structBuffer);
			if(!structBuffer.checkFinalSize()) {
				return 1;
			}
			writeJson(file, 0xFFFFFF);
		}

		if(fileFormat == AFF_Complex) {
			AUCTION_FILE file;
			if(!AuctionWriter::deserialize(&file, data)) {
				Object::logStatic(Object::LL_Error, "main", "Can't deserialize file %s\n", filename);
				return 3;
			}
			writeJson(file, file.header.file_version);
		} else {
			AUCTION_SIMPLE_FILE file;
			if(!AuctionWriter::deserialize(&file, data)) {
				Object::logStatic(Object::LL_Error, "main", "Can't deserialize file %s\n", filename);
				return 3;
			}
			writeJson(file, file.header.file_version);
		}
	}

	Object::logStatic(Object::LL_Info, "main", "Processed %d files\n", i - 1);

	return 0;
}
