#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sqlite3.h>
#include <map>
#include <vector>
#include "Core/CharsetConverter.h"

#pragma pack(push, 1)
struct ItemData {
	uint32_t handle;
	int32_t code;
	int64_t uid;
	int64_t count;
	int32_t ethereal_durability;
	uint32_t endurance;
	uint8_t enhance;
	uint8_t level;
	int32_t flag;
	int32_t socket[4];
	uint32_t awaken_option_value[5];
	int32_t awaken_option_data[5];
	int32_t remain_time;
	uint8_t elemental_effect_type;
	int32_t elemental_effect_remain_time;
	int32_t elemental_effect_attack_point;
	int32_t elemental_effect_magic_point;
	int32_t appearance_code;
	int32_t unknown1[51];
	int16_t unknown2;
};

struct AuctionDataEnd {
	int8_t duration_type;
	int64_t bid_price;
	int64_t price;
	char seller[31];
	int8_t flag;
};

struct AuctionInfo {
	int32_t size;
	int16_t version;
	int16_t flag;
	int64_t time;
	int64_t previousTime;
	int16_t category;
	int32_t uid;
};
#pragma pack(pop)

enum DiffType {
	D_Added = 0,
	D_Updated = 1,
	D_Deleted = 2,
	D_Unmodified = 4,
	D_Unrecognized = 1000
};

int main(int argc, char* argv[]) {
	const int totalRecognizedSize = sizeof(struct AuctionInfo) + sizeof(struct ItemData) + sizeof(struct AuctionDataEnd);
	std::map<uint32_t, std::vector<uint8_t>> auctions;

	if(argc < 2) {
		printf("Record size is %d(0x%08X)\n", totalRecognizedSize, totalRecognizedSize);
		printf("Usage: %s auctions.bin\n", argv[0]);
		return 0;
	}

	int i;
	for(i = 1; i < argc; i++) {
		const char* filename = argv[i];
		char outputFilename[512];

		sprintf(outputFilename, "%s.txt", filename);

		FILE* file = fopen(filename, "rb");
		if(!file) {
			printf("Cant open file %s\n", filename);
			return 1;
		}

		while(!feof(file)) {
			struct AuctionInfo header;
			char buffer[1024];

			if(fread(&header, sizeof(header), 1, file) != 1)
				break;

			int dataSize = header.size - sizeof(header);
			if(fread(buffer, 1, dataSize, file) != dataSize)
				break;


			switch(header.flag) {
				case D_Added: {
					auto it = auctions.insert(std::pair<uint32_t, std::vector<uint8_t>>(header.uid, std::vector<uint8_t>(buffer, buffer + dataSize)));
					if(it.second == false) {
						fprintf(stderr, "Error: added auction already exists: 0x%08X\n", header.uid);
					}
					break;
				}
				case D_Updated: {
					auto it = auctions.find(header.uid);
					if(it == auctions.end()) {
						fprintf(stderr, "Error: updated auction not found: 0x%08X\n", header.uid);
					} else {
						it->second = std::vector<uint8_t>(buffer, buffer + dataSize);
					}
					break;
				}
				case D_Deleted: {
					int erased = auctions.erase(header.uid);
					if(erased != 1) {
						fprintf(stderr, "Error: deleted auction not found: 0x%08X, found %d\n", header.uid, erased);
					}
					break;
				}
				default:
					fprintf(stderr, "Error: unsupported flag %d for auction 0x%08X\n", header.flag, header.uid);
					break;
			}
		}

		fclose(file);
	}

	fprintf(stderr, "Processed %d files\n", i-1);

	return 0;
}
