#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1

#define TS_CS_AUCTION_INSTANT_PURCHASE_DEF(_) \
	_(simple)(int32_t, auction_uid)

#define TS_CS_AUCTION_INSTANT_PURCHASE_ID(X) \
	X(1308, version < EPIC_9_6_3) \
	X(2308, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_CS_AUCTION_INSTANT_PURCHASE, SessionType::GameClient, SessionPacketOrigin::Client);
#undef TS_CS_AUCTION_INSTANT_PURCHASE_DEF

