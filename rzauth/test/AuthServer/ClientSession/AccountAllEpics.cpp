#include "../GameServerSession/Common.h"
#include "../GlobalConfig.h"
#include "AuthGame/TS_AG_CLIENT_LOGIN.h"
#include "NetSession/AutoClientSession.h"
#include "PacketEnums.h"
#include "RzTest.h"
#include "gtest/gtest.h"
#include <stdlib.h>

namespace AuthServer {

class GameSession : public ClientAuthSession {
public:
	GameSession(int epic, std::function<void(uint64_t)> onLoggedIn, std::function<void(void)> onError)
	    : ClientAuthSession(nullptr, epic),
	      selectedServer(false),
	      onLoggedIn(std::move(onLoggedIn)),
	      onError(std::move(onError)) {
		log(LL_Info, "Testing epic %x\n", epic);
	}

protected:
	virtual void onAuthDisconnected(bool causedByRemote) override {
		EXPECT_FALSE(causedByRemote);
		EXPECT_TRUE(selectedServer);
		if(causedByRemote) {
			log(LL_Error, "Unexpected disconnection from Auth (maybe wrong version ?)\n");
			onError();
		} else {
			onLoggedIn(getOnTimePassword());
		}
	}

	virtual void onAuthResult(TS_ResultCode result, const std::string& resultString) override {
		EXPECT_EQ(result, TS_RESULT_SUCCESS);

		if(result != TS_RESULT_SUCCESS) {
			log(LL_Error,
			    "%s: Auth failed result: %d (%s)\n",
			    getAccountName().c_str(),
			    result,
			    resultString.empty() ? "no associated string" : resultString.c_str());
			abortSession();
			onError();
		} else {
			log(LL_Info, "Retrieving server list\n");
			retreiveServerList();
		}
	}
	virtual void onServerList(const std::vector<ServerInfo>& servers, uint16_t lastSelectedServerId) override {
		const ServerInfo* serverInfo;

		EXPECT_EQ(2, servers.size());

		if(servers.size() != 2) {
			abortSession();
			onError();
			return;
		}

		EXPECT_TRUE(servers[0].serverId == 13 || servers[0].serverId == 3);
		EXPECT_TRUE(servers[1].serverId == 13 || servers[1].serverId == 3);

		if(servers[0].serverId == 13)
			serverInfo = &servers[0];
		else
			serverInfo = &servers[1];

		EXPECT_EQ(13, serverInfo->serverId);
		EXPECT_STREQ("Server 10", serverInfo->serverName.c_str());
		EXPECT_STREQ("127.0.0.10", serverInfo->serverIp.c_str());
		EXPECT_EQ(4710, serverInfo->serverPort);
		EXPECT_EQ(0, serverInfo->userRatio);

		if(servers[0].serverId == 3)
			serverInfo = &servers[0];
		else
			serverInfo = &servers[1];

		EXPECT_EQ(3, serverInfo->serverId);
		EXPECT_STREQ("Server 1", serverInfo->serverName.c_str());
		EXPECT_STREQ("127.0.0.1", serverInfo->serverIp.c_str());
		EXPECT_EQ(4610, serverInfo->serverPort);
		EXPECT_EQ(0, serverInfo->userRatio);

		log(LL_Info, "Selecting server 3\n");
		selectedServer = true;
		selectServer(3);
	}

	virtual void onGameDisconnected(bool causedByRemote) override {}
	virtual void onGameResult(TS_ResultCode result) override {}

private:
	bool selectedServer;
	std::function<void(uint64_t)> onLoggedIn;
	std::function<void(void)> onError;
};

class TS_CA_ACCOUNT_AllEpicsTest : public testing::TestWithParam<int> {
public:
	virtual void SetUp() {}
	virtual void TearDown() {}
};

INSTANTIATE_TEST_SUITE_P(AllEpics,
                         TS_CA_ACCOUNT_AllEpicsTest,
                         testing::Values(EPIC_2,
                                         EPIC_3,
                                         EPIC_4_1,
                                         EPIC_4_1_1,
                                         EPIC_5_1,
                                         EPIC_5_2,
                                         EPIC_6_1,
                                         EPIC_6_2,
                                         EPIC_6_3,
                                         EPIC_7_1,
                                         EPIC_7_2,
                                         EPIC_7_3,
                                         EPIC_7_4,
                                         EPIC_8_1,
                                         EPIC_8_1_1_RSA,
                                         EPIC_8_2,
                                         EPIC_8_3,
                                         EPIC_9_1,
                                         EPIC_9_2,
                                         EPIC_9_3,
                                         EPIC_9_4,
                                         EPIC_9_4_AR,
                                         EPIC_9_4_2,
                                         EPIC_9_5,
                                         EPIC_9_5_1,
                                         EPIC_9_5_2,
                                         EPIC_9_5_3,
                                         EPIC_9_6,
                                         EPIC_9_6_1,
                                         EPIC_9_6_2,
                                         EPIC_9_6_3,
                                         EPIC_9_6_4,
                                         // EPIC_9_6_5, // This version can't be detected by rzauth
                                         EPIC_9_6_6,
                                         EPIC_9_6_7,
                                         EPIC_LATEST));

TEST_P(TS_CA_ACCOUNT_AllEpicsTest, valid) {
	RzTest test;

	TestConnectionChannel game1(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);
	TestConnectionChannel game2(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	GameSession gameSession(
	    GetParam(),
	    [&game2](uint64_t otp) { sendClientLogin(&game2, "test1", otp); },
	    [&game1, &game2]() {
		    game1.abortTest();
		    game2.abortTest();
	    });

	game1.start();

	addGameLoginScenario(
	    game1,
	    13,
	    "Server 10",
	    "http://www.example.com/index10.html",
	    true,
	    "127.0.0.10",
	    4710,
	    [&game2](TestConnectionChannel* channel, TestConnectionChannel::Event event) { game2.start(); });

	addGameLoginScenario(game2,
	                     3,
	                     "Server 1",
	                     "http://www.example.com/index1.html",
	                     false,
	                     "127.0.0.1",
	                     4610,
	                     [&gameSession](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		                     gameSession.connect(CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, "test1", "admin");
	                     });

	game2.addCallback([&game1](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_AG_CLIENT_LOGIN* packet = AGET_PACKET(TS_AG_CLIENT_LOGIN);

		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);
		EXPECT_STREQ("test1", packet->account);
		EXPECT_EQ(1, packet->nAccountID);
		EXPECT_EQ(0, packet->nPCBangUser);
		EXPECT_EQ(0, packet->nEventCode);
		EXPECT_EQ(19, packet->nAge);
		//		EXPECT_EQ(0, packet->nContinuousPlayTime);
		//		EXPECT_EQ(0, packet->nContinuousLogoutTime);

		sendClientLogout(channel, "test1");
		channel->closeSession();
		game1.closeSession();
	});

	game1.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	game2.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	test.addChannel(&game1);
	test.addChannel(&game2);

	test.run(5000, [&gameSession]() { gameSession.closeSession(); });
}

}  // namespace AuthServer
