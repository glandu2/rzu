#pragma once

#include "Packet/PacketDeclaration.h"

// Last tested: EPIC_9_8_1

#define TS_CA_VERSION_DEF(_) \
	_(string)(szVersion, 20)
CREATE_PACKET(TS_CA_VERSION, 10001, SessionType::AuthClient, SessionPacketOrigin::Client);

