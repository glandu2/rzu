#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_SC_UNMOUNT_SUMMON_DEF(_) \
	_(simple)(ar_handle_t, handle) \
	_(simple)(ar_handle_t, summon_handle) \
	_(simple)(int8_t, flag)

CREATE_PACKET(TS_SC_UNMOUNT_SUMMON, 321);
#undef TS_SC_UNMOUNT_SUMMON_DEF

