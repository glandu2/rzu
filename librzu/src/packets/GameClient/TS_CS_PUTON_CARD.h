#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_CS_PUTON_CARD_DEF(_) \
	_(def)(simple)(int32_t, position) \
	_(impl)(simple)(int8_t, position, version < EPIC_9_6_7) \
	_(simple)(ar_handle_t, item_handle) \
	_(impl)(simple)(int32_t, position, version >= EPIC_9_6_7)

#define TS_CS_PUTON_CARD_ID(X) \
	X(214, version < EPIC_9_6_3) \
	X(1214, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_CS_PUTON_CARD, SessionType::GameClient, SessionPacketOrigin::Client);
#undef TS_CS_PUTON_CARD_DEF

