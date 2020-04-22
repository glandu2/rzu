#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_CS_PUTON_ITEM_DEF(_) \
	_(simple) (int8_t, position) \
	_(simple) (ar_handle_t, item_handle) \
	_(simple) (ar_handle_t, target_handle)

CREATE_PACKET(TS_CS_PUTON_ITEM, 200);
#undef TS_CS_PUTON_ITEM_DEF

