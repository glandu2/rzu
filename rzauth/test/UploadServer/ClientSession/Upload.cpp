#include "gtest/gtest.h"
#include "RzTest.h"
#include "../GlobalConfig.h"
#include "PacketEnums.h"
#include "UploadGame/TS_SU_LOGIN.h"
#include "UploadGame/TS_US_LOGIN_RESULT.h"
#include "UploadGame/TS_SU_REQUEST_UPLOAD.h"
#include "UploadGame/TS_US_REQUEST_UPLOAD.h"
#include "UploadGame/TS_US_UPLOAD.h"
#include "UploadClient/TS_CU_LOGIN.h"
#include "UploadClient/TS_CU_UPLOAD.h"
#include "UploadClient/TS_UC_LOGIN_RESULT.h"
#include "UploadClient/TS_UC_UPLOAD.h"
#include "Core/Utils.h"

#include "JpegImage.h"

namespace UploadServer {

TEST(TS_CU_UPLOAD, valid) {
	RzTest test;
	TestConnectionChannel upload(TestConnectionChannel::Client, CONFIG_GET()->upload.ip, CONFIG_GET()->upload.port, true);
	TestConnectionChannel game(TestConnectionChannel::Client, CONFIG_GET()->game.ip, CONFIG_GET()->game.port, false);

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		TS_SU_LOGIN loginPacket;
		TS_MESSAGE::initMessage(&loginPacket);

		strcpy(loginPacket.server_name, "game001");
		channel->sendPacket(&loginPacket);
	});

	game.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_US_LOGIN_RESULT* packet = AGET_PACKET(TS_US_LOGIN_RESULT);

		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);

		TS_SU_REQUEST_UPLOAD uploadPacket;
		TS_MESSAGE::initMessage(&uploadPacket);

		uploadPacket.client_id = 0;
		uploadPacket.account_id = 1;
		uploadPacket.guild_sid = 2;
		uploadPacket.one_time_password = 3;
		uploadPacket.type = 4;

		channel->sendPacket(&uploadPacket);
	});

	game.addCallback([&upload](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_US_REQUEST_UPLOAD* packet = AGET_PACKET(TS_US_REQUEST_UPLOAD);

		EXPECT_EQ(TS_RESULT_SUCCESS, packet->result);

		upload.start();
	});

	upload.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		TS_CU_LOGIN loginPacket;
		TS_MESSAGE::initMessage(&loginPacket);

		loginPacket.client_id = 0;
		loginPacket.account_id = 1;
		loginPacket.guild_id = 2;
		loginPacket.one_time_password = 3;
		strcpy(loginPacket.raw_server_name, "game001");

		channel->sendPacket(&loginPacket);
	});

	upload.addCallback([&game](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_UC_LOGIN_RESULT* packet = AGET_PACKET(TS_UC_LOGIN_RESULT);
		ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);

		TS_CU_UPLOAD* uploadPacket = TS_MESSAGE_WNA::create<TS_CU_UPLOAD, char>(sizeof(ONE_PIXEL_IMAGE));
		uploadPacket->file_length = sizeof(ONE_PIXEL_IMAGE);
		memcpy((char*)uploadPacket->file_contents, ONE_PIXEL_IMAGE, sizeof(ONE_PIXEL_IMAGE));

		channel->sendPacket(uploadPacket);
		TS_MESSAGE_WNA::destroy(uploadPacket);

		channel->closeSession();
	});


	game.addCallback([&upload](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		const TS_US_UPLOAD* packet = AGET_PACKET(TS_US_UPLOAD);

		struct tm timeinfo;
		Utils::getGmTime(time(NULL), &timeinfo);
		char expectedFilename[128];
		sprintf(expectedFilename, "game001_0000000002_%02d%02d%02d_%02d%02d%02d.jpg",
				 timeinfo.tm_year % 100,
				 timeinfo.tm_mon,
				 timeinfo.tm_mday,
				 timeinfo.tm_hour,
				 timeinfo.tm_min,
				 timeinfo.tm_sec);


		EXPECT_EQ(2, packet->guild_id);
		EXPECT_EQ(sizeof(ONE_PIXEL_IMAGE), packet->file_size);
		EXPECT_EQ(36, (int)packet->filename_length);
		EXPECT_EQ(0, packet->type);
		EXPECT_STREQ(expectedFilename, std::string(packet->file_name, packet->filename_length).c_str());
		EXPECT_EQ(sizeof(*packet) + packet->filename_length, packet->size);

		channel->closeSession();
	});

	game.start();
	test.addChannel(&game);
	test.addChannel(&upload);
	test.run();
}

} // namespace UploadServer
