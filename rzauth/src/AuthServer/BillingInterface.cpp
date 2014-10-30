#include "BillingInterface.h"
#include "GameServerSession.h"
#include "ClientData.h"

static const char MSG_UNKNOWN_COMMAND[] = "Unknown command\r\n";

#ifdef __GLIBC__
#include <malloc.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <crtdbg.h>
#include <malloc.h>
#endif

namespace AuthServer {

void BillingInterface::onCommand(const std::vector<std::string>& args) {
	if(args.size() == 0)
		return;

	if(args.size() >= 3 && args[0] == "billing_notice")
		billingNotice(args[1], args[2]);
	else {
		debug("Unknown billing command: %s\n", args[0].c_str());
		debug("Usage: billing_notice blank <account_id>\n");
	}
}

void BillingInterface::billingNotice(const std::string& cmd, const std::string& accountIdStr) {
	uint32_t accountId = atoi(accountIdStr.c_str());

	if(cmd == "blank") {
		AuthServer::ClientData* client = AuthServer::ClientData::getClientById(accountId);
		if(client && client->getGameServer() && client->isConnectedToGame()) {
			client->getGameServer()->sendNotifyItemPurchased(client);
			debug("Billing notice for client %s (id: %d)\n", client->account.c_str(), client->accountId);
		} else {
			debug("Billing notice for a not in-game client: %d\n", accountId);
		}
	} else {
		debug("Billing notice error: only \"blank\" is supported. Received: %s\n", cmd.c_str());
	}
}

} // namespace AuthServer
