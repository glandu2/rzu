#include "BillingInterface.h"
#include "GameData.h"
#include "ClientData.h"
#include <stdlib.h>

namespace AuthServer {

static const char MSG_CONNECTED[] = "Connected to billing telnet server\r\n";

void BillingInterface::onConnected() {
	write(MSG_CONNECTED, sizeof(MSG_CONNECTED));
}

void BillingInterface::onCommand(const std::vector<std::string>& args) {
	if(args.size() == 0)
		return;

	if( args[0] == "billing_notify") {
		if(args.size() >= 3) {
			billingNotice(args[1], args[2]);
		} else {
			debug("Command billing_notify: expected 2 arguments, got %d\n", (int)args.size() - 1);
		}
	} else {
		debug("Unknown billing command: %s\n", args[0].c_str());
		debug("Usage: billing_notify blank <account_id>\n");
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
