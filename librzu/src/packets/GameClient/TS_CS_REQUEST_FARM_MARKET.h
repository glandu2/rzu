#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1

#define TS_CS_REQUEST_FARM_MARKET_DEF(_)

// Since EPIC_7_3
#define TS_CS_REQUEST_FARM_MARKET_ID(X) \
	X(6008, true)

CREATE_PACKET_VER_ID(TS_CS_REQUEST_FARM_MARKET, SessionType::GameClient, SessionPacketOrigin::Client);
#undef TS_CS_REQUEST_FARM_MARKET_DEF

