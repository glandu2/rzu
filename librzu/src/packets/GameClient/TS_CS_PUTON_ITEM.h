#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1

#define TS_CS_PUTON_ITEM_DEF(_) \
	_(def)(simple) (int8_t, position) \
	_(impl)(simple) (int8_t, position, version < EPIC_9_6_7) \
	_(simple) (ar_handle_t, item_handle) \
	_(simple) (ar_handle_t, target_handle) \
	_(impl)(simple) (int8_t, position, version >= EPIC_9_6_7)

#define TS_CS_PUTON_ITEM_ID(X) \
	X(200, version < EPIC_9_6_3) \
	X(1200, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_CS_PUTON_ITEM, SessionType::GameClient, SessionPacketOrigin::Client);
#undef TS_CS_PUTON_ITEM_DEF

