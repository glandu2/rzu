#ifndef AUTHBENCHMARK_H
#define AUTHBENCHMARK_H

#include "RappelzSocket.h"
#include "ICallbackGuard.h"
#include "uv.h"
#include "Authentication.h"
#include "Packets/PacketEnums.h"

class RappelzSocket;

class AuthBenchmark : private ICallbackGuard
{
public:
	AuthBenchmark();

protected:
	static void onAuthResult(void* instance, Authentication* auth, TS_ErrorCode result, const char* resultString);
	static void onServerList(void* instance, Authentication* auth, const std::vector<Authentication::ServerInfo>& servers);
	static void onGameResult(void* instance, Authentication* auth, TS_ErrorCode result, RappelzSocket* gameServerSocket);

private:
	Authentication* auth;
};

#endif // AUTHBENCHMARK_H
