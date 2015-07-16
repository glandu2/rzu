#ifndef LOGSERVER_LS_11N4S_H
#define LOGSERVER_LS_11N4S_H

#include <stdint.h>

#pragma pack(push, 1)
struct LS_11N4S
{
	uint16_t id;
	uint16_t size;
	uint8_t type;
	uint32_t thread_id;

	int64_t n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11;

	uint16_t len1;
	uint16_t len2;
	uint16_t len3;
	uint16_t len4;
};
#pragma pack(pop)

#endif // LOGSERVER_LS_11N4S_H
