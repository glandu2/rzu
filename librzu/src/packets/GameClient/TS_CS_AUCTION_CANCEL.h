#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_CS_AUCTION_CANCEL_DEF(_) \
	_(simple)(uint32_t, auction_uid)

CREATE_PACKET(TS_CS_AUCTION_CANCEL, 1310);
#undef TS_CS_AUCTION_CANCEL_DEF

