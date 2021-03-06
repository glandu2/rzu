#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_SERVER_INFO_DEF(_) \
	_(simple) (uint16_t, server_idx) \
	_(string) (server_name, 21) \
	_(simple) (bool, is_adult_server, version >= EPIC_4_1, false) \
	_(string) (server_screenshot_url, 256, version >= EPIC_4_1, "about:blank") \
	_(string) (server_ip, 16) \
	_(simple) (int32_t, server_port) \
	_(simple) (uint16_t, user_ratio)

CREATE_STRUCT(TS_SERVER_INFO);

#define TS_AC_SERVER_LIST_DEF(_) \
	_(simple)   (uint16_t, last_login_server_idx, version >= EPIC_4_1, 1) \
	_(count)    (uint16_t, servers) \
	_(dynarray) (TS_SERVER_INFO, servers)

CREATE_PACKET(TS_AC_SERVER_LIST, 10022, SessionType::AuthClient, SessionPacketOrigin::Server);


