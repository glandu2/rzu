#ifndef AUTHSERVER_BILLINGINTERFACE_H
#define AUTHSERVER_BILLINGINTERFACE_H

#include "Core/Object.h"
#include "NetSession/TelnetSession.h"
#include <string>

namespace AuthServer {

class BillingInterface : public TelnetSession
{
	DECLARE_CLASS(AuthServer::BillingInterface)
protected:
	EventChain<SocketSession> onConnected();
	void onCommand(const std::vector<std::string>& args);

	void billingNotice(const std::string& cmd, const std::string& accountIdStr);
};

} // namespace AuthServer

#endif // AUTHSERVER_BILLINGINTERFACE_H
