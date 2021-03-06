#pragma once

#include "Packet/PacketBaseMessage.h"

#pragma pack(push, 1)
struct TS_GA_ACCOUNT_LIST : public TS_MESSAGE_WNA
{
	struct AccountInfo {
		char account[61];
		int32_t nAccountID;
		char nPCBangUser;
		int32_t nEventCode;
		int32_t nAge;
		char ip[INET6_ADDRSTRLEN];
		uint32_t loginTime;
	};

	bool final_packet;
	uint8_t count;
	AccountInfo accountInfo[];

	static const uint16_t packetID = 60003;
};

#pragma pack(pop)

