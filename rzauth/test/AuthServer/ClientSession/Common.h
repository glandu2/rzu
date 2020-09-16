#pragma once

#include "AuthClient/Flat/TS_AC_AES_KEY_IV.h"
#include "AuthClient/Flat/TS_CA_ACCOUNT.h"
#include "TestConnectionChannel.h"
#include <stdint.h>

struct rsa_st;
typedef struct rsa_st RSA;

namespace AuthServer {

void sendVersion(TestConnectionChannel* channel, const char* versionStr = "201501120");
void sendAccountDES(TestConnectionChannel* channel, const char* account, const char* password);
void sendAccountDESDouble(TestConnectionChannel* channel, const char* account, const char* password);

RSA* createRSAKey();
void freeRSAKey(RSA* rsaCipher);
void sendRSAKey(RSA* rsaCipher, TestConnectionChannel* channel, uint16_t id = 0);
void parseAESKey(RSA* rsaCipher, const TS_AC_AES_KEY_IV* packet, unsigned char aes_key_iv[]);
template<typename PacketType>
void prepareAccountRSAPacket(unsigned char aes_key_iv[32],
                             PacketType* accountMsg,
                             const char* account,
                             const char* password);
void sendAccountRSA(unsigned char aes_key_iv[32],
                    TestConnectionChannel* channel,
                    const char* account,
                    const char* password);

void sendAccountIMBC(TestConnectionChannel* channel, const char* account, const char* password);
void sendAccountIMBCDouble(TestConnectionChannel* channel, const char* account, const char* password);
void sendAccountIMBC_RSA(unsigned char aes_key_iv[32],
                         TestConnectionChannel* channel,
                         const char* account,
                         const char* password);

void expectAuthResult(TestConnectionChannel::Event& event, uint16_t result, int32_t login_flag);

enum AuthMethod { AM_Des, AM_Aes };

void addClientLoginToServerListScenario(TestConnectionChannel& auth,
                                        AuthMethod method,
                                        const char* account,
                                        const char* password,
                                        unsigned char* aesKey = nullptr,
                                        const char* version = "201501120");

}  // namespace AuthServer
