#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1

#define MOVE_REQUEST_INFO_DEF(_) \
	_(simple)(float, tx) \
	_(simple)(float, ty)

CREATE_STRUCT(MOVE_REQUEST_INFO);
#undef MOVE_REQUEST_INFO_DEF

#define TS_CS_MOVE_REQUEST_DEF(_) \
	_(simple)(ar_handle_t, handle) \
	_(simple)(float, x) \
	_(simple)(float, y) \
	_(simple)(ar_time_t, cur_time) \
	_(simple)(uint8_t, speed_sync) \
	_(count) (uint16_t, move_infos) \
	_(dynarray)(MOVE_REQUEST_INFO, move_infos) \
	_(endarray)(uint8_t, dummy_padding)

#define TS_CS_MOVE_REQUEST_ID(X) \
	X(5, version < EPIC_9_2) \
	X(65, version >= EPIC_9_2 && version < EPIC_9_4_2) \
	X(63, version >= EPIC_9_4_2 && version < EPIC_9_6_3) \
	X(1063, version >= EPIC_9_6_3) \

CREATE_PACKET_VER_ID(TS_CS_MOVE_REQUEST, SessionType::GameClient, SessionPacketOrigin::Client);
#undef TS_CS_MOVE_REQUEST_DEF

