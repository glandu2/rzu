#pragma once

#include "Packet/PacketBaseMessage.h"

#pragma pack(push, 1)
struct TS_AC_AES_KEY_IV : public TS_MESSAGE_WNA
{
	int data_size;
	unsigned char rsa_encrypted_data[0];
	static const uint16_t packetID = 72;
	static const uint16_t packetID2 = 1072;
};
#pragma pack(pop)

