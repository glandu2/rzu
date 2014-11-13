#ifndef ADMINSERVER_BILLINGINTERFACE_H
#define ADMINSERVER_BILLINGINTERFACE_H

#include "Object.h"
#include "../TelnetSession.h"
#include <string>

namespace AuthServer {

class BillingInterface : public TelnetSession
{
	DECLARE_CLASS(AuthServer::BillingInterface)
protected:
	void onConnected();
	void onCommand(const std::vector<std::string>& args);

	void billingNotice(const std::string& cmd, const std::string& accountIdStr);
};

} // namespace AuthServer

#endif // ADMINSERVER_BILLINGINTERFACE_H
