#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1

#define TS_CS_GET_BOOTHS_NAME_DEF(_) \
	_(count)(int32_t, handles) \
	_(dynarray)(ar_handle_t, handles)

#define TS_CS_GET_BOOTHS_NAME_ID(X) \
	X(707, version < EPIC_9_6_3) \
	X(1707, version >= EPIC_9_6_3)

CREATE_PACKET_VER_ID(TS_CS_GET_BOOTHS_NAME, SessionType::GameClient, SessionPacketOrigin::Client);
#undef TS_CS_GET_BOOTHS_NAME_DEF

