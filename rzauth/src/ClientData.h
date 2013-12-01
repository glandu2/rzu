#ifndef CLIENTDATA_H
#define CLIENTDATA_H

#include <string>
#include <stdint.h>

struct ClientData {
	std::string account;
	uint32_t accountId;
	uint32_t age;
	uint16_t lastLoginServerId;
	uint32_t eventCode;
	uint64_t oneTimePassword;
};

#endif // CLIENTDATA_H
