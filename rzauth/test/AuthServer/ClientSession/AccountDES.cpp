#include "gtest/gtest.h"
#include "RzTest.h"
#include "../GlobalConfig.h"
#include "AuthClient/Flat/TS_CA_ACCOUNT.h"
#include "AuthClient/Flat/TS_AC_RESULT.h"
#include "AuthClient/Flat/TS_AC_RESULT_WITH_STRING.h"
#include "PacketEnums.h"
#include "Common.h"
#include "Database/DbQueryJobRef.h"
#include "Database/DbConnectionPool.h"

#include "Cipher/DesPasswordCipher.h"

/*
 * Double login
 * Duplicate account (kick)
 */

struct SqlTest
{
	struct Input {
		int32_t account_id;
	};

	struct Output {
		std::string account_name;
	};
};

template<> void DbQueryJob<SqlTest>::init(DbConnectionPool* dbConnectionPool) {
	createBinding(dbConnectionPool,
	              CONFIG_GET()->connectionString,
	              "select account from account where account_id = ?;",
	              DbQueryBinding::EM_OneRow);

	addParam("account_id", &InputType::account_id);
	addColumn("account", &OutputType::account_name);
}
DECLARE_DB_BINDING(SqlTest, "sql_test");

namespace AuthServer {

TEST(TS_CA_ACCOUNT_DES, valid) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		AuthServer::sendAccountDES(channel, "test1", "admin");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_SUCCESS, TS_AC_RESULT::LSF_EULA_ACCEPTED);
		channel->closeSession();
	});

	auth.start();
	test.addChannel(&auth);
	test.run();
}


TEST(TS_CA_ACCOUNT_DES, db_accent_utf8) {
	SqlTest::Input testInput;
	bool ok = false;

	testInput.account_id = 21;

	DbConnectionPool dbConnectionPool;
	DbBindingLoader::get()->initAll(&dbConnectionPool);

	class SqlTestQuery : public DbQueryJob<SqlTest> {
	public:
		SqlTestQuery(bool& ok) : ok(ok) {}
		bool& ok;
		virtual void onDone(Status) {
			ok = true;
			auto& results = getResults();
			ASSERT_EQ(1, results.size());
			ASSERT_STREQ("test\xE9", results[0].get()->account_name.c_str());
		}
	};
	DbQueryJob<SqlTest>* query = new SqlTestQuery(ok);
	query->execute(&testInput, 1);
	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
	ASSERT_EQ(true, ok);
}

TEST(TS_CA_ACCOUNT_DES, double_account_before_result) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);
	bool receivedFirst = false;
	bool firstIsOk;

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		AuthServer::sendAccountDESDouble(channel, "test1", "admin");
	});

	auth.addCallback([&receivedFirst, &firstIsOk](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Packet, event.type);
		EXPECT_TRUE(event.packet->id == TS_AC_RESULT::packetID || event.packet->id == TS_AC_RESULT_WITH_STRING::packetID);

		if(event.packet->id == TS_AC_RESULT::packetID) {
			const TS_AC_RESULT* packet = AGET_PACKET(TS_AC_RESULT);

			if((receivedFirst && firstIsOk) || (!receivedFirst && packet->result == TS_RESULT_CLIENT_SIDE_ERROR)) {
				ASSERT_EQ(TS_RESULT_CLIENT_SIDE_ERROR, packet->result);
				EXPECT_EQ(0, packet->login_flag);
				if(!receivedFirst)
					firstIsOk = false;
			} else {
				ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);
				EXPECT_EQ(TS_AC_RESULT::LSF_EULA_ACCEPTED, packet->login_flag);
				if(!receivedFirst)
					firstIsOk = true;
			}
		} else if(event.packet->id == TS_AC_RESULT_WITH_STRING::packetID) {
			const TS_AC_RESULT_WITH_STRING* packet = AGET_PACKET(TS_AC_RESULT_WITH_STRING);

			if((receivedFirst && firstIsOk) || (!receivedFirst && packet->result == TS_RESULT_CLIENT_SIDE_ERROR)) {
				ASSERT_EQ(TS_RESULT_MISC, packet->result);
				EXPECT_EQ(0, packet->login_flag);
				if(!receivedFirst)
					firstIsOk = false;
			} else {
				ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);
				EXPECT_EQ(TS_AC_RESULT::LSF_EULA_ACCEPTED, packet->login_flag);
				if(!receivedFirst)
					firstIsOk = true;
			}
		}
		if(receivedFirst)
			channel->closeSession();
		else
			receivedFirst = true;
	});

	auth.addCallback([&receivedFirst, &firstIsOk](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Packet, event.type);
		EXPECT_TRUE(event.packet->id == TS_AC_RESULT::packetID || event.packet->id == TS_AC_RESULT_WITH_STRING::packetID);

		if(event.packet->id == TS_AC_RESULT::packetID) {
			const TS_AC_RESULT* packet = AGET_PACKET(TS_AC_RESULT);

			if((receivedFirst && firstIsOk) || (!receivedFirst && packet->result == TS_RESULT_CLIENT_SIDE_ERROR)) {
				ASSERT_EQ(TS_RESULT_CLIENT_SIDE_ERROR, packet->result);
				EXPECT_EQ(0, packet->login_flag);
				if(!receivedFirst)
					firstIsOk = false;
			} else {
				ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);
				EXPECT_EQ(TS_AC_RESULT::LSF_EULA_ACCEPTED, packet->login_flag);
				if(!receivedFirst)
					firstIsOk = true;
			}
		} else if(event.packet->id == TS_AC_RESULT_WITH_STRING::packetID) {
			const TS_AC_RESULT_WITH_STRING* packet = AGET_PACKET(TS_AC_RESULT_WITH_STRING);

			if((receivedFirst && firstIsOk) || (!receivedFirst && packet->result == TS_RESULT_CLIENT_SIDE_ERROR)) {
				ASSERT_EQ(TS_RESULT_MISC, packet->result);
				EXPECT_EQ(0, packet->login_flag);
				if(!receivedFirst)
					firstIsOk = false;
			} else {
				ASSERT_EQ(TS_RESULT_SUCCESS, packet->result);
				EXPECT_EQ(TS_AC_RESULT::LSF_EULA_ACCEPTED, packet->login_flag);
				if(!receivedFirst)
					firstIsOk = true;
			}
		}
		if(receivedFirst)
			channel->closeSession();
		else
			receivedFirst = true;
	});

	auth.start();
	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_ACCOUNT_DES, double_account_after_result) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		AuthServer::sendAccountDES(channel, "test1", "admin");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_SUCCESS, TS_AC_RESULT::LSF_EULA_ACCEPTED);
		AuthServer::sendAccountDES(channel, "test1", "admin");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_CLIENT_SIDE_ERROR, 0);
		channel->closeSession();
	});

	auth.start();
	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_ACCOUNT_DES, duplicate_auth_account_connection) {
	RzTest test;
	TestConnectionChannel auth1(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);
	TestConnectionChannel auth2(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth1.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		AuthServer::sendAccountDES(channel, "test1", "admin");
	});

	auth1.addCallback([&auth2](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_SUCCESS, TS_AC_RESULT::LSF_EULA_ACCEPTED);
		auth2.start();
	});

	auth2.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		AuthServer::sendAccountDES(channel, "test1", "admin");
	});

	auth1.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		EXPECT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	auth2.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_ALREADY_EXIST, 0);
		channel->closeSession();
	});

	auth1.start();
	test.addChannel(&auth1);
	test.addChannel(&auth2);
	test.run();
}

TEST(TS_CA_ACCOUNT_DES, valid_disconect_before_result) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		AuthServer::sendAccountDES(channel, "test1", "admin");
		channel->closeSession();
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
	});

	auth.start();
	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_ACCOUNT_DES, invalid_account) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		AuthServer::sendAccountDES(channel, "invalidaccount", "admin");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	auth.start();
	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_ACCOUNT_DES, invalid_password) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);
		AuthServer::sendAccountDES(channel, "test1", "invalid_password");
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	auth.start();
	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_ACCOUNT_DES, garbage_data) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);

		TS_CA_ACCOUNT accountPacket;
		TS_MESSAGE::initMessage(&accountPacket);

		memset(accountPacket.account, 127, sizeof(accountPacket.account));
		memset(accountPacket.password, 127, sizeof(accountPacket.password));

		channel->sendPacket(&accountPacket);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	auth.start();
	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_ACCOUNT_DES, garbage_data_epic4) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);

		TS_CA_ACCOUNT_EPIC4 accountPacket;
		TS_MESSAGE::initMessage(&accountPacket);

		memset(accountPacket.account, 127, sizeof(accountPacket.account));
		memset(accountPacket.password, 127, sizeof(accountPacket.password));

		channel->sendPacket(&accountPacket);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	auth.start();
	test.addChannel(&auth);
	test.run();
}

TEST(TS_CA_ACCOUNT_DES, garbage_data_before_des) {
	RzTest test;
	TestConnectionChannel auth(TestConnectionChannel::Client, CONFIG_GET()->auth.ip, CONFIG_GET()->auth.port, true);

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		AuthServer::sendVersion(channel);

		TS_CA_ACCOUNT accountPacket;
		TS_MESSAGE::initMessage(&accountPacket);

		strcpy(accountPacket.account, "test1");
		memset(accountPacket.password, 127, sizeof(accountPacket.password));
		DesPasswordCipher("MERONG").encrypt(accountPacket.password, sizeof(accountPacket.password));

		channel->sendPacket(&accountPacket);
	});

	auth.addCallback([](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		AuthServer::expectAuthResult(event, TS_RESULT_NOT_EXIST, 0);
		channel->closeSession();
	});

	auth.start();
	test.addChannel(&auth);
	test.run();
}

} // namespace AuthServer
