#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1

#define TS_CA_SERVER_LIST_DEF(_)
CREATE_PACKET(TS_CA_SERVER_LIST, 10021, SessionType::AuthClient, SessionPacketOrigin::Client);

