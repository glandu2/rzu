#ifndef AUTHBENCHMARK_H
#define AUTHBENCHMARK_H

#include "RappelzSocket.h"
#include "IListener.h"
#include "uv.h"
#include "Authentication.h"
#include "Packets/PacketEnums.h"

class RappelzSocket;

class AuthBenchmark : private IListener
{
public:
	AuthBenchmark();

protected:
	static void onAuthResult(IListener* instance, Authentication* auth, TS_ResultCode result, const char* resultString);
	static void onServerList(IListener* instance, Authentication* auth, const std::vector<Authentication::ServerInfo>& servers);
	static void onGameResult(IListener* instance, Authentication* auth, TS_ResultCode result, RappelzSocket* gameServerSocket);

private:
	Authentication* auth;
};

#endif // AUTHBENCHMARK_H
