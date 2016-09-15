#include "Environment.h"
#include "AuthServer/GlobalConfig.h"
#include "Database/DbConnectionPool.h"
#include "Database/DbConnection.h"

Environment* Environment::instance = nullptr;

void Environment::beforeTests() {
	std::string authExec = CONFIG_GET()->authExecutable.get();
	std::string gameReconnectExec = CONFIG_GET()->gameReconnectExecutable.get();
	std::string connectionString = CONFIG_GET()->connectionString.get();

	DbConnectionPool dbConnectionPool;
	DbConnection* connection = dbConnectionPool.getConnection(connectionString.c_str());
	ASSERT_NE(nullptr, connection);
	ASSERT_NE(false, connection->execute("DROP TABLE IF EXISTS account;"));
	ASSERT_NE(false, connection->execute("CREATE TABLE account (\r\n"
	                    "        \"account_id\"    INTEGER NOT NULL,\r\n"
	                    "        \"account\"       VARCHAR(61) NOT NULL,\r\n"
	                    "        \"password\"      VARCHAR(61),\r\n"
	                    "        \"last_login_server_idx\" INTEGER,\r\n"
	                    "        \"server_idx_offset\"     INTEGER,\r\n"
	                    "        \"security_no\"   VARCHAR(61),\r\n"
	                    "        PRIMARY KEY(account_id)\r\n);"));
	connection->setAutoCommit(false);
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(0,'test0','613b5247e3398350918cb622a3ec19e9',NULL,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(1,'test1','613b5247e3398350918cb622a3ec19e9',NULL,NULL,'613b5247e3398350918cb622a3ec19e9');"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(2,'test2','613b5247e3398350918cb622a3ec19e9',4,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(3,'test3','613b5247e3398350918cb622a3ec19e9',NULL,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(4,'test4','613b5247e3398350918cb622a3ec19e9',NULL,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(5,'test5','613b5247e3398350918cb622a3ec19e9',11,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(6,'test6','613b5247e3398350918cb622a3ec19e9',12,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(7,'test7','613b5247e3398350918cb622a3ec19e9',NULL,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(8,'test8','613b5247e3398350918cb622a3ec19e9',NULL,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(9,'test9','613b5247e3398350918cb622a3ec19e9',NULL,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(10,'test10','613b5247e3398350918cb622a3ec19e9',NULL,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(11,'test11','613b5247e3398350918cb622a3ec19e9',NULL,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(12,'test12','613b5247e3398350918cb622a3ec19e9',NULL,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(13,'test13','613b5247e3398350918cb622a3ec19e9',NULL,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(14,'test14','613b5247e3398350918cb622a3ec19e9',NULL,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(15,'test15','613b5247e3398350918cb622a3ec19e9',NULL,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(16,'test16','613b5247e3398350918cb622a3ec19e9',NULL,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(17,'test17','613b5247e3398350918cb622a3ec19e9',NULL,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(18,'test18','613b5247e3398350918cb622a3ec19e9',NULL,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(19,'test19','613b5247e3398350918cb622a3ec19e9',NULL,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(20,'test20','613b5247e3398350918cb622a3ec19e9',NULL,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(21,'test' || char(0xE9),'613b5247e3398350918cb622a3ec19e9',NULL,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(1000000001,'testPw47Chars','c8d8079110d491e6d115dc0755c7e5eb',NULL,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(1000000002,'testPw60Chars','33410d89b4a115d9ac9c7aaaff255b91',NULL,NULL,NULL);"));
	ASSERT_NE(false, connection->execute("INSERT INTO account VALUES(1000000003,'testPw64Chars','0504f832c91bc39001f67f4209f7f077',NULL,NULL,NULL);"));
	connection->endTransaction(true);
	connection->releaseAndClose();

	std::string connectionStringArg = "/auth.db.connectionstring:";
	connectionStringArg += connectionString;

	spawnProcess(4500, authExec.c_str(), 2, "/configfile:./auth-test.opt", connectionStringArg.c_str());
	if(testGameReconnect)
		spawnProcess(4802, gameReconnectExec.c_str(), 1, "/configfile:./rzgamereconnect-test.opt");
}

void Environment::afterTests() {
	stop(4501);
	if(testGameReconnect)
		stop(4801);
}

bool Environment::isGameReconnectBeingTested()
{
	if(instance && instance->testGameReconnect)
		return true;
	return false;
}
