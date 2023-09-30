#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1

#define TS_CS_CANCEL_ACTION_DEF(_) \
	_(simple)(ar_handle_t, handle)

#define TS_CS_CANCEL_ACTION_ID(X) \
	X(150, version < EPIC_9_6_3) \
	X(1150, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_CS_CANCEL_ACTION, SessionType::GameClient, SessionPacketOrigin::Client);
#undef TS_CS_CANCEL_ACTION_DEF

