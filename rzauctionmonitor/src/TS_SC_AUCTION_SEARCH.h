#pragma once

#include "Packet/PacketBaseMessage.h"

#pragma pack(push, 1)
struct TS_AUCTION_INFO {
	uint32_t uid;
};

struct TS_SC_AUCTION_SEARCH : public TS_MESSAGE {
	int page_num;
	int total_page_count;
	int auction_info_count;
	char auctionInfos[];
	// 40 * sizeof(auctionInfo)

	static const int packetID = 1301;
};
#pragma pack(pop)

