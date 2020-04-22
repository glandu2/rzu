#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_SC_COMPETE_COUNTDOWN_DEF(_) \
	_(simple)(int8_t, compete_type) \
	_(string)(competitor, 31) \
	_(simple)(ar_handle_t, handle_competitor)

CREATE_PACKET(TS_SC_COMPETE_COUNTDOWN, 4504);
#undef TS_SC_COMPETE_COUNTDOWN_DEF

