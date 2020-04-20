#ifndef PACKETS_TS_CS_SET_MAIN_TITLE_H
#define PACKETS_TS_CS_SET_MAIN_TITLE_H

#include "Packet/PacketDeclaration.h"

#define TS_CS_SET_MAIN_TITLE_DEF(_) \
	_(simple)(int32_t, code)

// Since EPIC_8_1
CREATE_PACKET(TS_CS_SET_MAIN_TITLE, 628);
#undef TS_CS_SET_MAIN_TITLE_DEF

#endif // PACKETS_TS_CS_SET_MAIN_TITLE_H
