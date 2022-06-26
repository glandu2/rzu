#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_UC_UPLOAD_DEF(_) \
	_(simple)(uint16_t, result)

CREATE_PACKET(TS_UC_UPLOAD, 50008, SessionType::UploadClient, SessionPacketOrigin::Server);
