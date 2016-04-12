#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sqlite3.h>
#include <map>
#include <vector>
#include <openssl/sha.h>

#pragma pack(push, 1)
struct AuctionInfo {
	int32_t size;
	int16_t version;
	int16_t flag;
	int64_t time;
	int64_t previousTime;
	int16_t category;
};
#pragma pack(pop)

enum DiffType {
	D_Added = 0,
	D_Updated = 1,
	D_Deleted = 2,
	D_Unmodified = 3,
	D_Base = 4,
	D_Unrecognized = 1000
};

int main(int argc, char* argv[]) {
	std::map<uint32_t, std::vector<uint8_t>> auctions;

	if(argc < 2) {
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
			union Data {
				int32_t uid;
				char buffer[1024];
			} data;

			if(fread(&header, sizeof(header), 1, file) != 1)
				break;

			size_t dataSize = header.size - sizeof(header);
			if(dataSize > sizeof(data.buffer)) {
				fprintf(stderr, "Error: %s data size too large: %d at offset %d\n", filename, (int)dataSize, (int)(ftell(file) - sizeof(header)));
				break;
			}

			if(fread(data.buffer, 1, dataSize, file) != dataSize)
				break;

			switch(header.flag) {
				case D_Base:
				case D_Added: {
					auto it = auctions.insert(std::pair<uint32_t, std::vector<uint8_t>>(data.uid, std::vector<uint8_t>(data.buffer, data.buffer + dataSize)));
					if(it.second == false) {
						fprintf(stderr, "Error: %s added auction already exists: 0x%08X\n", filename, data.uid);
					}
					break;
				}
				case D_Updated: {
					auto it = auctions.find(data.uid);
					if(it == auctions.end()) {
						fprintf(stderr, "Error: %s updated auction not found: 0x%08X\n", filename, data.uid);
					} else {
						it->second = std::vector<uint8_t>(data.buffer, data.buffer + dataSize);
					}
					break;
				}
				case D_Deleted: {
					int erased = auctions.erase(data.uid);
					if(erased != 1) {
						fprintf(stderr, "Error: %s deleted auction not found: 0x%08X, found %d\n", filename, data.uid, erased);
					}
					break;
				}
				default:
					fprintf(stderr, "Error: %s unsupported flag %d for auction 0x%08X\n", filename, header.flag, data.uid);
					break;
			}
		}

		fclose(file);
	}

	auto it = auctions.begin();
	for(; it != auctions.end(); ++it) {
		const std::vector<uint8_t>& data = it->second;
		unsigned char shaData[SHA_DIGEST_LENGTH];
		SHA(&data[0], data.size(), shaData);
		printf("0x%08X: ", it->first);
		for(int i = 0; i < SHA_DIGEST_LENGTH; i++) {
			printf("%02X", shaData[i]);
		}
		fputc('\n', stdout);
	}

	fprintf(stderr, "Processed %d files\n", i-1);

	return 0;
}
