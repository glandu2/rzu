#include "../GlobalConfig.h"
#include "PacketEnums.h"
#include "RzTest.h"
#include "UploadClient/flat/TS_CU_LOGIN.h"
#include "UploadClient/flat/TS_CU_UPLOAD.h"
#include "UploadClient/flat/TS_UC_LOGIN_RESULT.h"
#include "UploadClient/flat/TS_UC_UPLOAD.h"
#include "gtest/gtest.h"

/*
 * Double login
 * Duplicate account (kick)
 */

namespace UploadServer {

TEST(TS_CU_LOGIN, no_request) {
	RzTest test;
	TestConnectionChannel upload(
	    TestConnectionChannel::Client, CONFIG_GET()->upload.ip, CONFIG_GET()->upload.port, true);

	upload.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		TS_CU_LOGIN loginPacket;
		TS_MESSAGE::initMessage(&loginPacket);

		loginPacket.client_id = 0;
		loginPacket.account_id = 0;
		loginPacket.guild_id = 0;
		loginPacket.one_time_password = 0;
		memset(loginPacket.raw_server_name, 0x55, sizeof(loginPacket.raw_server_name));

		channel->sendPacket(&loginPacket);
	});

	upload.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_UC_LOGIN_RESULT* packet = AGET_PACKET(TS_UC_LOGIN_RESULT);
		EXPECT_EQ(TS_RESULT_NOT_EXIST, packet->result);
		channel->closeSession();
	});

	upload.start();
	test.addChannel(&upload);
	test.run();
}

}  // namespace UploadServer
