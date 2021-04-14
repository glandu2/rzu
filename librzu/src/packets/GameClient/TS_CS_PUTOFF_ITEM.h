#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_CS_PUTOFF_ITEM_DEF(_) \
	_(def)(simple) (int32_t, position) \
	_(impl)(simple)(int32_t, position, version >= EPIC_9_6_7) \
	_(impl)(simple)(int8_t, position, version < EPIC_9_6_7) \
	_(simple) (ar_handle_t, target_handle)

#define TS_CS_PUTOFF_ITEM_ID(X) \
	X(201, version < EPIC_9_6_3) \
	X(1201, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_CS_PUTOFF_ITEM, SessionType::GameClient, SessionPacketOrigin::Client);
#undef TS_CS_PUTOFF_ITEM_DEF

