#include "uv.h"
#include "BenchmarkAuthSession.h"
#include <string.h>
#include "EventLoop.h"
#include "LibRzuInit.h"
#include <stdio.h>
#include "ConfigInfo.h"
#include "GlobalCoreConfig.h"
#include "TimingFunctions.h"

void onSocketStateChange(IListener* instance, Stream *socket, Stream::State oldState, Stream::State newState, bool causedByRemote);

BenchmarkConfig config;
std::vector<BenchmarkAuthSession*> auths;

std::vector<Stream*> sockets; //for connections/sec test

bool benchConnections = false;

std::string ip;
int port;
bool useLocalhost;

static void init() {
	CFG_CREATE("ip", "127.0.0.1");
	CFG_CREATE("port", 4500);
	CFG_CREATE("account", "test");
	CFG_CREATE("count", 8);
	CFG_CREATE("targetcount", 3000);
	CFG_CREATE("password", "admin");
	CFG_CREATE("use_rsa", false);
	CFG_CREATE("delay", 0);
	CFG_CREATE("idxoffset", 0);
	CFG_CREATE("usecperconnection", 0);
	CFG_CREATE("benchconnection", false);
	GlobalCoreConfig::get()->app.appName.setDefault("rzbenchauth");
	GlobalCoreConfig::get()->app.configfile.setDefault("benchmarkauth.opt");
	GlobalCoreConfig::get()->log.level.setDefault("fatal");
	GlobalCoreConfig::get()->log.consoleLevel.setDefault("info");
	GlobalCoreConfig::get()->log.file.setDefault("benchmarkauth.log");
}

static std::string getIpForConnection(const std::string& originalIp, bool useLocalHost, int connection) {
	if(useLocalHost) {
		char buffer[20];
		sprintf(buffer, "127.0.0.%d", int(connection / 50000) + 1);

		return std::string(buffer);
	} else {
		return originalIp;
	}
}
void benchmarkAuthentication() {
	int count = CFG_GET("count")->getInt();

	config.port = CFG_GET("port")->getInt();
	config.connectionTargetCount = CFG_GET("targetcount")->getInt();
	config.delay = CFG_GET("delay")->getInt();
	config.method = CFG_GET("use_rsa")->getBool()? ClientAuthSession::ACM_RSA_AES : ClientAuthSession::ACM_DES;
	config.version = "200701120";

	auths.reserve(count);
	for(int i = 0; i < count; i++) {
		BenchmarkAuthSession* auth = new BenchmarkAuthSession(&config);
		auths.push_back(auth);
	}
}

void startBenchAuth(int usecBetweenConnection) {
	std::string accountNamePrefix = CFG_GET("account")->getString();
	std::string ip = CFG_GET("ip")->getString();
	int idxoffset = CFG_GET("idxoffset")->getInt();
	bool useLocalHost = ip == "127.0.0.1";

	for(size_t i = 0; i < auths.size(); i++) {
		const std::string account = (auths.size() > 1)? accountNamePrefix + std::to_string((long long)i + idxoffset) : accountNamePrefix;

		auths[i]->connect(getIpForConnection(ip, useLocalHost, i), account, CFG_GET("password")->getString());
		if(usecBetweenConnection)
			tfMicroSleep(usecBetweenConnection);
	}
}

void benchmarkConnections() {
	int count = CFG_GET("count")->getInt();

	sockets.reserve(count);
	for(int i = 0; i < count; i++) {
		Stream* socket = new Socket(EventLoop::getLoop(), false);
		socket->addEventListener(nullptr, &onSocketStateChange);

		sockets.push_back(socket);
	}
}

void startBenchConnections(int usecBetweenConnection) {
	ip = CFG_GET("ip")->getString();
	port = CFG_GET("port")->getInt();
	useLocalhost = ip == "127.0.0.1";

	for(size_t i = 0; i < sockets.size(); i++) {
		Stream* socket = sockets[i];
		socket->connect(ip, port);
		config.connectionsStarted++;
		if(usecBetweenConnection)
			tfMicroSleep(usecBetweenConnection);
	}
}

int main(int argc, char *argv[])
{
	LibRzuInit();
	init();
	ConfigInfo::get()->init(argc, argv);
	ConfigInfo::get()->dump();
	initTimer();

	Log mainLogger(GlobalCoreConfig::get()->log.enable,
				   GlobalCoreConfig::get()->log.level,
				   GlobalCoreConfig::get()->log.consoleLevel,
				   GlobalCoreConfig::get()->log.dir,
				   GlobalCoreConfig::get()->log.file,
				   GlobalCoreConfig::get()->log.maxQueueSize);
	Log::setDefaultLogger(&mainLogger);

	int usecBetweenConnection = CFG_GET("usecperconnection")->getInt();

	benchConnections = CFG_GET("benchconnection")->getBool();

	config.connectionsDone = config.connectionsStarted = 0;
	config.connectionTargetCount = CFG_GET("targetcount")->getInt();

	if(benchConnections)
		benchmarkConnections();
	else
		benchmarkAuthentication();

	mainLogger.log(Log::LL_Info, "main", 4, "Starting benchmark\n");

	if(usecBetweenConnection == 0)
		resetTimer();

	if(benchConnections)
		startBenchConnections(usecBetweenConnection);
	else
		startBenchAuth(usecBetweenConnection);

	if(usecBetweenConnection != 0) {
		mainLogger.log(Log::LL_Info, "main", 4, "Connected %d connections at limited speed, continuing benchmark at full-speed (time counter begin now)\n", config.connectionsStarted);
		resetTimer();
	}

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);

	unsigned long long int duration = getTimerValue();

	mainLogger.log(Log::LL_Info, "main", 4, "%d connections in %llu usec => %f auth/sec\n", config.connectionsDone, duration, config.connectionsDone/((float)duration/1000000.0f));
}

//conn bench
void onSocketStateChange(IListener* instance, Stream* socket, Stream::State oldState, Stream::State newState, bool causedByRemote) {
	if(newState == Stream::ConnectedState) {
		socket->close();
	} else if(newState == Stream::UnconnectedState) {
		config.connectionsDone++;
		if(config.connectionsStarted < config.connectionTargetCount) {
			config.connectionsStarted++;
			socket->connect(ip, port);
		}
	}
}
