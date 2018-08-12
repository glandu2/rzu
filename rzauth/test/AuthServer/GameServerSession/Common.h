#ifndef AUTHSERVER_GAMESERVERSESSION_COMMON_H
#define AUTHSERVER_GAMESERVERSESSION_COMMON_H

#include "TestConnectionChannel.h"
#include <stdint.h>
#include <vector>

namespace AuthServer {

void sendGameLogin(TestConnectionChannel* channel,
                   uint16_t index,
                   const char* name,
                   const char* screenshot,
                   bool isAdult,
                   const char* ip,
                   int32_t port);
void sendGameLoginWithLogout(TestConnectionChannel* channel,
                             uint16_t index,
                             const char* name,
                             const char* screenshot,
                             bool isAdult,
                             const char* ip,
                             int32_t port);
void sendGameLoginWithLogoutEx(TestConnectionChannel* channel,
                               uint16_t index,
                               const char* name,
                               const char* screenshot,
                               bool isAdult,
                               const char* ip,
                               int32_t port,
                               int8_t guid);

struct AccountInfo {
	std::string account;
	int32_t nAccountID;
	char nPCBangUser;
	int32_t nEventCode;
	int32_t nAge;
	char ip[INET6_ADDRSTRLEN];
	uint32_t loginTime;
};

void sendGameConnectedAccounts(TestConnectionChannel* channel, std::vector<AccountInfo> accounts);
void sendGameLogout(TestConnectionChannel* channel);
void sendClientLogin(TestConnectionChannel* channel, const char* account, uint64_t oneTimePassword);
void sendClientLogout(TestConnectionChannel* channel, const char* account);

void addGameLoginScenario(TestConnectionChannel& game,
                          uint16_t index,
                          const char* name,
                          const char* screenshot,
                          bool isAdult,
                          const char* ip,
                          int32_t port,
                          TestConnectionChannel::EventCallback callback);

}  // namespace AuthServer

#endif  // AUTHSERVER_GAMESERVERSESSION_COMMON_H
