#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_CS_STOP_WATCH_BOOTH_DEF(_) \
	_(simple)(ar_handle_t, target)

CREATE_PACKET(TS_CS_STOP_WATCH_BOOTH, 704);
#undef TS_CS_STOP_WATCH_BOOTH_DEF

