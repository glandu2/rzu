#pragma once

#include "Packet/PacketDeclaration.h"
#include "TS_SC_INVENTORY.h"

// Last tested: EPIC_9_8_1
#define TS_BOOTH_TRADE_ITEM_INFO_DEF(_) \
	_(simple)(uint16_t, item_type, version >= EPIC_9_8_1) \
	_(def)(simple)(int64_t, gold) \
	_(impl)(simple)(int64_t, gold, version >= EPIC_9_8_1) \
	_(simple)(TS_ITEM_FIXED_BASE_INFO, item) \
	_(impl)(simple)(int64_t, gold, version < EPIC_9_8_1)
CREATE_STRUCT(TS_BOOTH_TRADE_ITEM_INFO);
#undef TS_BOOTH_TRADE_ITEM_INFO_DEF

#define TS_SC_BOOTH_TRADE_INFO_DEF(_) \
	_(simple)(ar_handle_t, target) \
	_(simple)(bool, is_sell) \
	_(count)(uint16_t, trades) \
	_(dynarray)(TS_BOOTH_TRADE_ITEM_INFO, trades)

#define TS_SC_BOOTH_TRADE_INFO_ID(X) \
	X(710, version < EPIC_9_6_3) \
	X(1710, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_SC_BOOTH_TRADE_INFO, SessionType::GameClient, SessionPacketOrigin::Server);
#undef TS_SC_BOOTH_TRADE_INFO_DEF

