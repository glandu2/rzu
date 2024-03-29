#pragma once

#include "Packet/PacketDeclaration.h"
#include "TS_SC_INVENTORY.h"

// Last tested: EPIC_9_8_1

#define TS_AUCTION_INFO_DEF(_) \
	_(simple) (int32_t , auction_uid) \
	_(simple) (TS_ITEM_FIXED_INFO, item_info) \
	_(simple) (uint8_t, duration_type) \
	_(simple) (uint64_t, bidded_price) \
	_(simple) (uint64_t, instant_purchase_price)
CREATE_STRUCT(TS_AUCTION_INFO);
#undef TS_AUCTION_INFO_DEF

#define TS_SEARCHED_AUCTION_INFO_DEF(_) \
	_(simple) (TS_AUCTION_INFO, auction_details) \
	_(string) (seller_name, 31) \
	_(simple) (uint8_t, flag)
CREATE_STRUCT(TS_SEARCHED_AUCTION_INFO);
#undef TS_SEARCHED_AUCTION_INFO_DEF

#define TS_SC_AUCTION_SEARCH_DEF(_) \
	_(simple)(int32_t, page_num) \
	_(simple)(int32_t, total_page_count) \
	_(simple)(int32_t, auction_info_count) \
	_(array) (TS_SEARCHED_AUCTION_INFO, auction_info, 40)

#define TS_SC_AUCTION_SEARCH_ID(X) \
	X(1301, version < EPIC_9_6_3) \
	X(2301, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_SC_AUCTION_SEARCH, SessionType::GameClient, SessionPacketOrigin::Server);
#undef TS_SC_AUCTION_SEARCH_DEF

