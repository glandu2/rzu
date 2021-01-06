#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_CA_SELECT_SERVER_DEF(_) \
	_(def)(simple) (uint32_t, server_idx) \
	_(impl)(simple) (uint16_t, server_idx, version <  EPIC_9_6_5) \
	_(impl)(simple) (uint32_t, server_idx, version >= EPIC_9_6_5)
CREATE_PACKET(TS_CA_SELECT_SERVER, 10023, SessionType::AuthClient, SessionPacketOrigin::Client);

