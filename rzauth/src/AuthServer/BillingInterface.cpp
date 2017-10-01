#include "BillingInterface.h"
#include "ClientData.h"
#include "GameData.h"
#include <stdlib.h>

namespace AuthServer {

static const char MSG_CONNECTED[] = "Connected to billing telnet server\r\n";

EventChain<SocketSession> BillingInterface::onConnected() {
	write(MSG_CONNECTED, sizeof(MSG_CONNECTED));

	return TelnetSession::onConnected();
}

void BillingInterface::onCommand(const std::vector<std::string>& args) {
	if(args.size() == 0)
		return;

	if(args[0] == "billing_notify") {
		if(args.size() >= 3) {
			billingNotice(args[1], args[2]);
		} else {
			log(LL_Debug, "Command billing_notify: expected 2 arguments, got %d\n", (int) args.size() - 1);
		}
	} else {
		log(LL_Debug, "Unknown billing command: %s\n", args[0].c_str());
		log(LL_Debug, "Usage: billing_notify blank <account_id>\n");
	}
}

void BillingInterface::billingNotice(const std::string& cmd, const std::string& accountIdStr) {
	uint32_t accountId = atoi(accountIdStr.c_str());

	if(cmd == "blank") {
		AuthServer::ClientData* client = AuthServer::ClientData::getClientById(accountId);
		if(client && client->getGameServer() && client->isConnectedToGame()) {
			client->getGameServer()->sendNotifyItemPurchased(client);
			log(LL_Debug, "Billing notice for client %s (id: %d)\n", client->account.c_str(), client->accountId);
		} else {
			log(LL_Debug, "Billing notice for a not in-game client: %d\n", accountId);
		}
	} else {
		log(LL_Debug, "Billing notice error: only \"blank\" is supported. Received: %s\n", cmd.c_str());
	}
}

}  // namespace AuthServer
