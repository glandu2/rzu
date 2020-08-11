#pragma once

#include "Packet/PacketBaseMessage.h"

#pragma pack(push, 1)
struct TS_CS_AUCTION_SEARCH_FLAT : public TS_MESSAGE {
	int category_id;
	int sub_category_id;
	char keyword[31];
	int page_num;
	bool is_equipable;
	static const int packetID = 2300;
};
#pragma pack(pop)

