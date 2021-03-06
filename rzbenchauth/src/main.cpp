#include "BenchmarkAuthSession.h"
#include "Config/ConfigInfo.h"
#include "Config/GlobalCoreConfig.h"
#include "Core/EventLoop.h"
#include "Core/Log.h"
#include "Core/PrintfFormats.h"
#include "Core/Utils.h"
#include "LibRzuInit.h"
#include "Packet/PacketEpics.h"
#include "Stream/Socket.h"
#include "TimingFunctions.h"
#include "uv.h"
#include <stdio.h>
#include <string.h>

void onSocketStateChange(
    IListener* instance, Stream* socket, Stream::State oldState, Stream::State newState, bool causedByRemote);

BenchmarkConfig config;
std::vector<BenchmarkAuthSession*> auths;

std::vector<Stream*> sockets;  // for connections/sec test

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
	CFG_CREATE("epic", EPIC_LATEST);
	CFG_CREATE("delay", 0);
	CFG_CREATE("recodelay", 0);
	CFG_CREATE("idxoffset", 0);
	CFG_CREATE("usecperconnection", 0);
	CFG_CREATE("benchconnection", false);
	GlobalCoreConfig::get()->log.level.setDefault("fatal");
	GlobalCoreConfig::get()->log.consoleLevel.setDefault("info");
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
	config.recoDelay = CFG_GET("recodelay")->getInt();
	config.epic = CFG_GET("epic")->getInt();

	if(count > config.connectionTargetCount)
		count = config.connectionTargetCount;

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
		const std::string account =
		    (auths.size() > 1) ? accountNamePrefix + Utils::convertToString((int) i + idxoffset) : accountNamePrefix;

		auths[i]->connect(getIpForConnection(ip, useLocalHost, (int) i), account, CFG_GET("password")->getString());
		if(usecBetweenConnection)
			tfMicroSleep(usecBetweenConnection);
	}
}

void benchmarkConnections() {
	int count = CFG_GET("count")->getInt();

	if(count > config.connectionTargetCount)
		count = config.connectionTargetCount;

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

int main(int argc, char* argv[]) {
	LibRzuScopedUse useLibRzu;
	init();
	ConfigInfo::get()->init(argc, argv);

	uint64_t startTime = 0;
	Log mainLogger(GlobalCoreConfig::get()->log.enable,
	               GlobalCoreConfig::get()->log.level,
	               GlobalCoreConfig::get()->log.consoleLevel,
	               GlobalCoreConfig::get()->log.dir,
	               GlobalCoreConfig::get()->log.file,
	               GlobalCoreConfig::get()->log.maxQueueSize);
	Log::setDefaultLogger(&mainLogger);

	ConfigInfo::get()->dump();

	int usecBetweenConnection = CFG_GET("usecperconnection")->getInt();

	benchConnections = CFG_GET("benchconnection")->getBool();

	config.connectionsDone = config.connectionsStarted = 0;
	config.connectionTargetCount = CFG_GET("targetcount")->getInt();

	if(benchConnections)
		benchmarkConnections();
	else
		benchmarkAuthentication();

	mainLogger.log(Object::LL_Info, "main", 4, "Starting benchmark\n");

	if(usecBetweenConnection == 0)
		startTime = uv_hrtime();

	if(benchConnections)
		startBenchConnections(usecBetweenConnection);
	else
		startBenchAuth(usecBetweenConnection);

	if(usecBetweenConnection != 0) {
		mainLogger.log(
		    Object::LL_Info,
		    "main",
		    4,
		    "Connected %d connections at limited speed, continuing benchmark at full-speed (time counter begin now)\n",
		    config.connectionsStarted);
		startTime = uv_hrtime();
	}

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);

	uint64_t duration = (uv_hrtime() - startTime) / 1000;  // nanosec to usec

	mainLogger.log(Object::LL_Info,
	               "main",
	               4,
	               "%d connections in %" PRIu64 " usec => %f auth/sec\n",
	               config.connectionsDone,
	               duration,
	               config.connectionsDone / ((float) duration / 1000000.0f));
}

// conn bench
void onSocketStateChange(
    IListener* instance, Stream* socket, Stream::State oldState, Stream::State newState, bool causedByRemote) {
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
