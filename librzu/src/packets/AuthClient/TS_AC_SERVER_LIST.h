#pragma once

#include "Packet/PacketDeclaration.h"

#define TS_SERVER_INFO_DEF(_) \
	_(def)(simple) (uint32_t, server_idx) \
	_(impl)(simple) (uint16_t, server_idx, version <  EPIC_9_6_5) \
	_(impl)(simple) (uint32_t, server_idx, version >= EPIC_9_6_5) \
	_(string) (server_name, 21) \
	_(simple) (bool, is_adult_server, version >= EPIC_4_1, false) \
	_(string) (server_screenshot_url, 256, version >= EPIC_4_1, "about:blank") \
	_(string) (server_ip, 16) \
	_(simple) (int32_t, server_port) \
	_(simple) (uint16_t, user_ratio)

CREATE_STRUCT(TS_SERVER_INFO);

#define TS_AC_SERVER_LIST_DEF(_) \
	_(def)(simple)   (uint32_t, last_login_server_idx) \
	_(impl)(simple)   (uint16_t, last_login_server_idx, version >= EPIC_4_1 && version < EPIC_9_6_5, 1) \
	_(impl)(simple)   (uint32_t, last_login_server_idx, version >= EPIC_9_6_5, 1) \
	_(def)(count)    (uint32_t, servers) \
	_(impl)(count)    (uint16_t, servers, version <  EPIC_9_6_5) \
	_(impl)(count)    (uint32_t, servers, version >= EPIC_9_6_5) \
	_(dynarray) (TS_SERVER_INFO, servers)

CREATE_PACKET(TS_AC_SERVER_LIST, 10022, SessionType::AuthClient, SessionPacketOrigin::Server);
