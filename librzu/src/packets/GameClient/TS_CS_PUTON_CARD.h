#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_CS_PUTON_CARD_DEF(_) \
	_(simple)(int8_t, position) \
	_(simple)(ar_handle_t, item_handle)

CREATE_PACKET(TS_CS_PUTON_CARD, 214);
#undef TS_CS_PUTON_CARD_DEF

