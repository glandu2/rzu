#include "Common.h"
#include "Packets/TS_GA_LOGIN.h"
#include "Packets/TS_GA_CLIENT_LOGIN.h"
#include "Packets/TS_GA_CLIENT_LOGOUT.h"
#include "Packets/TS_AG_LOGIN_RESULT.h"
#include "Packets/TS_GA_LOGOUT.h"
#include "Packets/TS_GA_ACCOUNT_LIST.h"
#include "Packets/PacketEnums.h"

namespace AuthServer {

void sendGameLogin(TestConnectionChannel* channel, uint16_t index, const char* name, const char* screenshot, bool isAdult, const char* ip, int32_t port) {
	TS_GA_LOGIN packet;
	TS_MESSAGE::initMessage(&packet);

	packet.server_idx = index;
	strcpy(packet.server_name, name);
	strcpy(packet.server_screenshot_url, screenshot);
	packet.is_adult_server = isAdult;
	strcpy(packet.server_ip, ip);
	packet.server_port = port;

	channel->sendPacket(&packet);
}

void sendGameLoginEx(TestConnectionChannel* channel, uint16_t index, const char* name, const char* screenshot, bool isAdult, const char* ip, int32_t port) {
	TS_GA_LOGIN_WITH_LOGOUT packet;
	TS_MESSAGE::initMessage(&packet);

	packet.server_idx = index;
	strcpy(packet.server_name, name);
	strcpy(packet.server_screenshot_url, screenshot);
	packet.is_adult_server = isAdult;
	strcpy(packet.server_ip, ip);
	packet.server_port = port;

	channel->sendPacket(&packet);
}

void sendGameLogout(TestConnectionChannel *channel) {
	TS_GA_LOGOUT logout;
	TS_MESSAGE::initMessage(&logout);
	channel->sendPacket(&logout);
	channel->closeSession();
}

void sendClientLogin(TestConnectionChannel *channel, const char* account, uint64_t oneTimePassword) {
	TS_GA_CLIENT_LOGIN packet;
	TS_MESSAGE::initMessage(&packet);

	strcpy(packet.account, account);
	packet.one_time_key = oneTimePassword;

	channel->sendPacket(&packet);
}

void sendClientLogout(TestConnectionChannel *channel, const char* account) {
	TS_GA_CLIENT_LOGOUT packet;
	TS_MESSAGE::initMessage(&packet);

	strcpy(packet.account, account);

	channel->sendPacket(&packet);
}

void addGameLoginScenario(TestConnectionChannel &game,
						  uint16_t index, const char* name, const char* screenshot, bool isAdult, const char* ip, int32_t port,
						  TestConnectionChannel::EventCallback callback)
{
	game.addCallback([index, name, screenshot, isAdult, ip, port](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendGameLogin(channel, index, name, screenshot, isAdult, ip, port);
	});

	game.addCallback([callback](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_LOGIN_RESULT* packet = AGET_PACKET(TS_AG_LOGIN_RESULT);

		EXPECT_EQ(TS_RESULT_SUCCESS, packet->result);

		callback(channel, event);
	});
}

void sendGameConnectedAccounts(TestConnectionChannel *channel, std::vector<AccountInfo> accounts) {
	static const int MAXCOUNT_PER_PACKET = 2;

	TS_GA_ACCOUNT_LIST* accountListPacket;
	int maxCount = accounts.size() <= MAXCOUNT_PER_PACKET ? accounts.size() : MAXCOUNT_PER_PACKET;
	accountListPacket = TS_MESSAGE_WNA::create<TS_GA_ACCOUNT_LIST, TS_GA_ACCOUNT_LIST::AccountInfo>(maxCount);

	auto it = accounts.begin();
	do {
		int count = 0;

		for(; it != accounts.end() && count < maxCount; ++it, ++count) {
			const AccountInfo& accountItem = *it;
			TS_GA_ACCOUNT_LIST::AccountInfo* accountInfo = &accountListPacket->accountInfo[count];

			strcpy(accountInfo->account, accountItem.account.c_str());
			accountInfo->nAccountID = accountItem.nAccountID;
			accountInfo->nPCBangUser = accountItem.nPCBangUser;
			accountInfo->nEventCode = accountItem.nEventCode;
			accountInfo->nAge = accountItem.nAge;
			accountInfo->ip = accountItem.ip;
			accountInfo->loginTime = accountItem.loginTime;
		}

		if(it == accounts.end())
			accountListPacket->final_packet = true;
		else
			accountListPacket->final_packet = false;
		accountListPacket->count = count;

		channel->sendPacket(accountListPacket);
	} while(it != accounts.end());

	TS_MESSAGE_WNA::destroy(accountListPacket);
}

} //namespace AuthServer
