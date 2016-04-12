#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <map>
#include <vector>
#include "Core/CharsetConverter.h"
#include "Database/DbQueryJobCallback.h"

cval<std::string>& connectionString = CFG_CREATE("connexionstring", "DRIVER=SQLite3 ODBC Driver;Database=auctions.sqlite3;");

struct DB_Item
{
	struct Input {
		int32_t uid;
		int16_t diff_flag;
		int64_t previous_time;
		int64_t time;
		int16_t category;
		int8_t duration_type;
		int64_t bid_price;
		int64_t price;
		std::string seller;
		int8_t bid_flag;

		uint32_t handle;
		int32_t code;
		int64_t item_uid;
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
	};

	struct Output {};
};

template<> void DbQueryJob<DB_Item>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
				  connectionString,
				  "INSERT INTO auctions ("
				  "\"uid\", "
				  "\"diff_flag\", "
				  "\"previous_time\", "
				  "\"time\", "
				  "\"category\", "
				  "\"duration_type\", "
				  "\"bid_price\", "
				  "\"price\", "
				  "\"seller\", "
				  "\"bid_flag\", "
				  "\"handle\", "
				  "\"code\", "
				  "\"item_uid\", "
				  "\"count\", "
				  "\"ethereal_durability\", "
				  "\"endurance\", "
				  "\"enhance\", "
				  "\"level\", "
				  "\"flag\", "
				  "\"socket_0\", "
				  "\"socket_1\", "
				  "\"socket_2\", "
				  "\"socket_3\", "
				  "\"awaken_option_value_0\", "
				  "\"awaken_option_value_1\", "
				  "\"awaken_option_value_2\", "
				  "\"awaken_option_value_3\", "
				  "\"awaken_option_value_4\", "
				  "\"awaken_option_data_0\", "
				  "\"awaken_option_data_1\", "
				  "\"awaken_option_data_2\", "
				  "\"awaken_option_data_3\", "
				  "\"awaken_option_data_4\", "
				  "\"remain_time\", "
				  "\"elemental_effect_type\", "
				  "\"elemental_effect_remain_time\", "
				  "\"elemental_effect_attack_point\", "
				  "\"elemental_effect_magic_point\", "
				  "\"appearance_code\") "
				  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);",
				  DbQueryBinding::EM_NoRow);

	addParam("uid", &InputType::uid);
	addParam("diff_flag", &InputType::diff_flag);
	addParam("previous_time", &InputType::previous_time);
	addParam("time", &InputType::time);
	addParam("category", &InputType::category);
	addParam("duration_type", &InputType::duration_type);
	addParam("bid_price", &InputType::bid_price);
	addParam("price", &InputType::price);
	addParam("seller", &InputType::seller);
	addParam("bid_flag", &InputType::bid_flag);
	addParam("handle", &InputType::handle);
	addParam("code", &InputType::code);
	addParam("item_uid", &InputType::item_uid);
	addParam("count", &InputType::count);
	addParam("ethereal_durability", &InputType::ethereal_durability);
	addParam("endurance", &InputType::endurance);
	addParam("enhance", &InputType::enhance);
	addParam("level", &InputType::level);
	addParam("flag", &InputType::flag);
	addParam("socket_0", &InputType::socket_0);
	addParam("socket_1", &InputType::socket_1);
	addParam("socket_2", &InputType::socket_2);
	addParam("socket_3", &InputType::socket_3);
	addParam("awaken_option_value_0", &InputType::awaken_option_value_0);
	addParam("awaken_option_value_1", &InputType::awaken_option_value_1);
	addParam("awaken_option_value_2", &InputType::awaken_option_value_2);
	addParam("awaken_option_value_3", &InputType::awaken_option_value_3);
	addParam("awaken_option_value_4", &InputType::awaken_option_value_4);
	addParam("awaken_option_data_0", &InputType::awaken_option_data_0);
	addParam("awaken_option_data_1", &InputType::awaken_option_data_1);
	addParam("awaken_option_data_2", &InputType::awaken_option_data_2);
	addParam("awaken_option_data_3", &InputType::awaken_option_data_3);
	addParam("awaken_option_data_4", &InputType::awaken_option_data_4);
	addParam("remain_time", &InputType::remain_time);
	addParam("elemental_effect_type", &InputType::elemental_effect_type);
	addParam("elemental_effect_remain_time", &InputType::elemental_effect_remain_time);
	addParam("elemental_effect_attack_point", &InputType::elemental_effect_attack_point);
	addParam("elemental_effect_magic_point", &InputType::elemental_effect_magic_point);
	addParam("appearance_code", &InputType::appearance_code);
}
DECLARE_DB_BINDING(AuthServer::DB_UpdateLastServerIdx, "db_updatelastserveridx");

#pragma pack(push, 1)
struct ItemData {
	uint32_t handle;
	int32_t code;
	int64_t item_uid;
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
	D_Unmodified = 3,
	D_Base = 4,
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

	sqlite3* db;
	sqlite3_stmt* stmt;
	int result;
	result = sqlite3_open_v2("auctions.sqlite3", &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, nullptr);
	if(result != SQLITE_OK) {
		fprintf(stderr, "Sqlite open error: %d\n", result);
		return 1;
	}
	sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);

	result = sqlite3_prepare_v2(db, "INSERT INTO auctions ("
						   "uid, "
						   "diff_flag, "
						   "previous_time, "
						   "\"time\", "
						   "category, "
						   "duration_type, "
						   "bid_price, "
						   "price, "
						   "seller, "
						   "bid_flag, "
						   "handle, "
						   "code, "
						   "item_uid, "
						   "\"count\", "
						   "ethereal_durability, "
						   "endurance, "
						   "enhance, "
						   "\"level\", "
						   "flag, "
						   "socket_0, "
						   "socket_1, "
						   "socket_2, "
						   "socket_3, "
						   "awaken_option_value_0, "
						   "awaken_option_value_1, "
						   "awaken_option_value_2, "
						   "awaken_option_value_3, "
						   "awaken_option_value_4, "
						   "awaken_option_data_0, "
						   "awaken_option_data_1, "
						   "awaken_option_data_2, "
						   "awaken_option_data_3, "
						   "awaken_option_data_4, "
						   "remain_time, "
						   "elemental_effect_type, "
						   "elemental_effect_remain_time, "
						   "elemental_effect_attack_point, "
						   "elemental_effect_magic_point, "
						   "appearance_code) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
					   -1,
					   &stmt,
					   nullptr);
	if(result != SQLITE_OK) {
		fprintf(stderr, "Sqlite prepare error: %d, %s\n", result, sqlite3_errmsg(db));
		return 1;
	}

	CharsetConverter cp1252ToUtf8("CP1252", "UTF-8");

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
			struct AuctionInfo auctionInfo = {0};
			struct ItemData itemData;
			struct AuctionDataEnd auctionDataEnd;

			if(fread(&auctionInfo, sizeof(auctionInfo), 1, file) != 1)
				break;

			if(fread(&itemData, sizeof(itemData), 1, file) != 1)
				break;

			int moveOffset = auctionInfo.size - (sizeof(auctionInfo) + sizeof(itemData) + sizeof(auctionDataEnd));
			if(moveOffset != 0) {
				fseek(file, moveOffset, SEEK_CUR);
			}

			if(fread(&auctionDataEnd, sizeof(auctionDataEnd), 1, file) != 1)
				break;

			int col = 1;

			std::string seller(auctionDataEnd.seller);
			std::string utf8Seller;
			cp1252ToUtf8.convert(seller, utf8Seller, 1);

			sqlite3_bind_int   (stmt, col++, auctionInfo.uid);
			sqlite3_bind_int   (stmt, col++, auctionInfo.flag);
			sqlite3_bind_int64 (stmt, col++, auctionInfo.previousTime);
			sqlite3_bind_int64 (stmt, col++, auctionInfo.time);
			sqlite3_bind_int   (stmt, col++, auctionInfo.category);
			sqlite3_bind_int   (stmt, col++, auctionDataEnd.duration_type);
			sqlite3_bind_int64 (stmt, col++, auctionDataEnd.bid_price);
			sqlite3_bind_int64 (stmt, col++, auctionDataEnd.price);
			sqlite3_bind_text  (stmt, col++, utf8Seller.c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_int   (stmt, col++, auctionDataEnd.flag);
			sqlite3_bind_int   (stmt, col++, itemData.handle                       );
			sqlite3_bind_int   (stmt, col++, itemData.code                         );
			sqlite3_bind_int64 (stmt, col++, itemData.uid                          );
			sqlite3_bind_int64 (stmt, col++, itemData.count                        );
			sqlite3_bind_int   (stmt, col++, itemData.ethereal_durability          );
			sqlite3_bind_int   (stmt, col++, itemData.endurance                    );
			sqlite3_bind_int   (stmt, col++, itemData.enhance                      );
			sqlite3_bind_int   (stmt, col++, itemData.level                        );
			sqlite3_bind_int   (stmt, col++, itemData.flag                         );
			sqlite3_bind_int   (stmt, col++, itemData.socket[0]                    );
			sqlite3_bind_int   (stmt, col++, itemData.socket[1]                    );
			sqlite3_bind_int   (stmt, col++, itemData.socket[2]                    );
			sqlite3_bind_int   (stmt, col++, itemData.socket[3]                    );
			sqlite3_bind_int   (stmt, col++, itemData.awaken_option_value[0]       );
			sqlite3_bind_int   (stmt, col++, itemData.awaken_option_value[1]       );
			sqlite3_bind_int   (stmt, col++, itemData.awaken_option_value[2]       );
			sqlite3_bind_int   (stmt, col++, itemData.awaken_option_value[3]       );
			sqlite3_bind_int   (stmt, col++, itemData.awaken_option_value[4]       );
			sqlite3_bind_int   (stmt, col++, itemData.awaken_option_data[0]        );
			sqlite3_bind_int   (stmt, col++, itemData.awaken_option_data[1]        );
			sqlite3_bind_int   (stmt, col++, itemData.awaken_option_data[2]        );
			sqlite3_bind_int   (stmt, col++, itemData.awaken_option_data[3]        );
			sqlite3_bind_int   (stmt, col++, itemData.awaken_option_data[4]        );
			sqlite3_bind_int   (stmt, col++, itemData.remain_time                  );
			sqlite3_bind_int   (stmt, col++, itemData.elemental_effect_type        );
			sqlite3_bind_int   (stmt, col++, itemData.elemental_effect_remain_time );
			sqlite3_bind_int   (stmt, col++, itemData.elemental_effect_attack_point);
			sqlite3_bind_int   (stmt, col++, itemData.elemental_effect_magic_point );
			sqlite3_bind_int   (stmt, col++, itemData.appearance_code              );

			result = sqlite3_step(stmt);
			if(result != SQLITE_DONE) {
				fprintf(stderr, "Sqlite step error uid: %d, time %d: %d, %s\n", auctionInfo.uid, (int)auctionInfo.time, result, sqlite3_errmsg(db));
				//return 1;
			}
			sqlite3_reset(stmt);
		}
/*
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
		}*/

		fclose(file);
	}

	sqlite3_finalize(stmt);
	sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
	sqlite3_close(db);


	fprintf(stderr, "Processed %d files\n", i-1);
/*
	auto it = auctions.begin();
	for(; it != auctions.end(); ++it) {
		const std::vector<uint8_t>& data = it->second;
		unsigned char shaData[SHA_DIGEST_LENGTH];
		SHA(&data[0], data.size(), shaData);
		printf("0x%08X: ", it->first);
		for(i = 0; i < SHA_DIGEST_LENGTH; i++) {
			printf("%02X", shaData[i]);
		}
		fputc('\n', stdout);
	}*/

	return 0;
}

/*
 * create table auctions (
uid integer NOT NULL,
diff_flag smallint NOT NULL,
"time" datetime NOT NULL,
category smallint NOT NULL,

duration_type smallint NOT NULL,
bid_price bigint NOT NULL,
price bigint NOT NULL,
seller[31] char NOT NULL,
bid_flag smallint NOT NULL,

handle integer NOT NULL,
code integer NOT NULL,
item_uid bigint NOT NULL,
"count" bigint NOT NULL,
ethereal_durability integer NOT NULL,
endurance integer NOT NULL,
enhance smallint NOT NULL,
"level" smallint NOT NULL,
flag integer NOT NULL,
socket_0 integer NOT NULL,
socket_1 integer NOT NULL,
socket_2 integer NOT NULL,
socket_3 integer NOT NULL,
awaken_option_value_0 integer NOT NULL,
awaken_option_value_1 integer NOT NULL,
awaken_option_value_2 integer NOT NULL,
awaken_option_value_3 integer NOT NULL,
awaken_option_value_4 integer NOT NULL,
awaken_option_data_0 integer NOT NULL,
awaken_option_data_1 integer NOT NULL,
awaken_option_data_2 integer NOT NULL,
awaken_option_data_3 integer NOT NULL,
awaken_option_data_4 integer NOT NULL,
remain_time integer NOT NULL,
elemental_effect_type smallint NOT NULL,
elemental_effect_remain_time integer NOT NULL,
elemental_effect_attack_point integer NOT NULL,
elemental_effect_magic_point integer NOT NULL,
appearance_code integer NOT NULL,
PRIMARY KEY (uid, "time")
)*/
