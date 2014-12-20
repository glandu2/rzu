#include "uv.h"
#include "Authentication.h"
#include <string.h>
#include "EventLoop.h"
#include "RappelzLibInit.h"
#include <stdio.h>
#include "PacketSession.h"
#include "ConfigInfo.h"
#include "RappelzLibConfig.h"
#include "TimingFunctions.h"

void onAuthResult(IListener* instance, Authentication* auth, TS_ResultCode result, const std::string &resultString);
void onAuthRetrieveServer(uv_timer_t* handle);
void onServerList(IListener* instance, Authentication* auth, const std::vector<Authentication::ServerInfo>* servers, uint16_t lastSelectedServerId);
void onGameResult(IListener* instance, Authentication* auth, TS_ResultCode result, PacketSession *gameServerSocket);
void onAuthClosed(IListener* instance, Authentication* auth);
void onAuthClosedWithFailure(IListener* instance, Authentication* auth);

void onSocketStateChange(IListener* instance, Stream *socket, Stream::State oldState, Stream::State newState);
void onSocketTimer(uv_timer_t *handle);
void onSocketClosed();

std::vector<std::string> accounts;
std::vector<Authentication*> auths;
std::vector<Stream*> sockets; //for connections/sec test
std::vector<uv_timer_t*> timers;
bool printDebug = false;
int connectionsStarted = 0;
int connectionsDone = 0;
int connectionTargetCount;
bool connectToGs = false;
int delay = 0;
bool benchConnections = false;

std::string ip;
int port;
bool useLocalhost;

static void init() {
	CFG_CREATE("ip", "127.0.0.1" /*"127.0.0.1"*/);
	CFG_CREATE("port", 4500);
	CFG_CREATE("account", "test");
	CFG_CREATE("count", 8);
	CFG_CREATE("targetcount", 3000);
	CFG_CREATE("password", "admin");
	CFG_CREATE("use_rsa", false);
	CFG_CREATE("printall", false);
	CFG_CREATE("connecttogs", false);
	CFG_CREATE("delay", 0);
	CFG_CREATE("idxoffset", 0);
	CFG_CREATE("usecperconnection", 0);
	CFG_CREATE("benchconnection", false);
	RappelzLibConfig::get()->app.appName.setDefault("RappelzBenchmarkAuth");
	RappelzLibConfig::get()->app.configfile.setDefault("benchmarkauth.opt");
	RappelzLibConfig::get()->log.level.setDefault("fatal");
	RappelzLibConfig::get()->log.consoleLevel.setDefault("info");
	RappelzLibConfig::get()->log.file.setDefault("benchmarkauth.log");
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
	std::string accountNamePrefix = CFG_GET("account")->getString();
	std::string ip = CFG_GET("ip")->getString();
	int port = CFG_GET("port")->getInt();
	bool usersa = CFG_GET("use_rsa")->getBool();
	int idxoffset = CFG_GET("idxoffset")->getInt();
	bool useLocalHost = ip == "127.0.0.1";

	accounts.reserve(count);
	auths.reserve(count);
	if(delay)
		timers.reserve(count);
	for(int i = 0; i < count; i++) {
		const std::string account = (count > 1)? accountNamePrefix + std::to_string((long long)i + idxoffset) : accountNamePrefix;

		Authentication* auth = new Authentication(getIpForConnection(ip, useLocalHost, i), usersa? Authentication::ACM_RSA_AES : Authentication::ACM_DES, port);
		auth->index = i;

		accounts.push_back(account);
		auths.push_back(auth);

		if(delay) {
			uv_timer_t* timer = new uv_timer_t;
			uv_timer_init(EventLoop::getLoop(), timer);
			timer->data = auth;
			timers.push_back(timer);
		}
	}
}

void startBenchAuth(int usecBetweenConnection) {
	for(size_t i = 0; i < auths.size(); i++) {
		auths[i]->connect(accounts[i], CFG_GET("password")->getString(), Callback<Authentication::CallbackOnAuthResult>(nullptr, &onAuthResult));
		connectionsStarted++;
		if(usecBetweenConnection)
			tfMicroSleep(usecBetweenConnection);
	}
}

void benchmarkConnections() {
	int count = CFG_GET("count")->getInt();

	sockets.reserve(count);
	if(delay)
		timers.reserve(count);
	for(int i = 0; i < count; i++) {
		Stream* socket = new Socket(EventLoop::getLoop(), false);
		socket->addEventListener(nullptr, &onSocketStateChange);

		sockets.push_back(socket);

		if(delay) {
			uv_timer_t* timer = new uv_timer_t;
			uv_timer_init(EventLoop::getLoop(), timer);
			timer->data = socket;
			timers.push_back(timer);
		}
	}
}

void startBenchConnections(int usecBetweenConnection) {
	ip = CFG_GET("ip")->getString();
	port = CFG_GET("port")->getInt();
	useLocalhost = ip == "127.0.0.1";

	for(size_t i = 0; i < sockets.size(); i++) {
		Stream* socket = sockets[i];
		socket->connect(ip, port);
		connectionsStarted++;
		if(usecBetweenConnection)
			tfMicroSleep(usecBetweenConnection);
	}
}

int main(int argc, char *argv[])
{
	RappelzLibInit();
	init();
	ConfigInfo::get()->init(argc, argv);
	ConfigInfo::get()->dump();
	initTimer();

	Log mainLogger(RappelzLibConfig::get()->log.enable,
				   RappelzLibConfig::get()->log.level,
				   RappelzLibConfig::get()->log.consoleLevel,
				   RappelzLibConfig::get()->log.dir,
				   RappelzLibConfig::get()->log.file,
				   RappelzLibConfig::get()->log.maxQueueSize);
	Log::setDefaultLogger(&mainLogger);

	int usecBetweenConnection = CFG_GET("usecperconnection")->getInt();

	connectionTargetCount = CFG_GET("targetcount")->getInt();
	printDebug = CFG_GET("printall")->getBool();
	connectToGs = CFG_GET("connecttogs")->getBool();
	delay = CFG_GET("delay")->getInt();
	benchConnections = CFG_GET("benchconnection")->getBool();

	if(benchConnections)
		benchmarkConnections();
	else
		benchmarkAuthentication();

	fprintf(stderr, "Starting benchmark\n");

	if(usecBetweenConnection == 0)
		resetTimer();

	if(benchConnections)
		startBenchConnections(usecBetweenConnection);
	else
		startBenchAuth(usecBetweenConnection);

	if(usecBetweenConnection != 0) {
		fprintf(stderr, "Connected %d connections at limited speed, continuing benchmark at full-speed (time counter begin now)\n", connectionsStarted);
		resetTimer();
	}

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);

	unsigned long long int duration = getTimerValue();

	fprintf(stderr, "%d connections in %llu usec => %f auth/sec\n", connectionsDone, duration, connectionsDone/((float)duration/1000000.0f));
}

//auth bench
void onAuthResult(IListener* instance, Authentication* auth, TS_ResultCode result, const std::string& resultString) {
	if(printDebug || result != TS_RESULT_SUCCESS)
		fprintf(stderr, "%s: Auth result: %d (%s)\n", auth->getAccountName().c_str(), result, resultString.empty() ? "no associated string" : resultString.c_str());
	if(result == TS_RESULT_SUCCESS) {
		if(delay == 0)
			auth->retreiveServerList(Callback<Authentication::CallbackOnServerList>(nullptr, &onServerList));
		else {
			uv_timer_start(timers[auth->index], &onAuthRetrieveServer, delay, 0);
		}
	} else {
		auth->abort(Callback<Authentication::CallbackOnAuthClosed>(nullptr, &onAuthClosedWithFailure));
	}
}

void onAuthRetrieveServer(uv_timer_t* handle) {
	Authentication* auth = (Authentication*) handle->data;

	auth->retreiveServerList(Callback<Authentication::CallbackOnServerList>(nullptr, &onServerList));
}

void onServerList(IListener* instance, Authentication* auth, const std::vector<Authentication::ServerInfo>* servers, uint16_t lastSelectedServerId) {
	if(printDebug) {
		fprintf(stderr, "%p Server list (last id: %d)\n", auth, lastSelectedServerId);
		for(size_t i = 0; i < servers->size(); i++) {
			fprintf(stderr, "%d: %20s at %16s:%d %d%% user ratio\n",
					servers->at(i).serverId,
					servers->at(i).serverName.c_str(),
					servers->at(i).serverIp.c_str(),
					servers->at(i).serverPort,
					servers->at(i).userRatio);
		}
	}
	if(connectToGs && servers->size() > 0)
		auth->selectServer(servers->at(0).serverId, Callback<Authentication::CallbackOnGameResult>(nullptr, &onGameResult));
	else
		auth->abort(Callback<Authentication::CallbackOnAuthClosed>(nullptr, &onAuthClosed));
}

void onAuthClosedWithFailure(IListener* instance, Authentication* auth) {
	if(printDebug)
		fprintf(stderr, "%p auth closed with failure\n", auth);
	//auth->connect(nullptr, CFG_GET("password")->getString(), Callback<Authentication::CallbackOnAuthResult>(nullptr, &onAuthResult));
}

void onAuthClosed(IListener* instance, Authentication* auth) {
	if(printDebug)
		fprintf(stderr, "%p auth closed\n", auth);
	connectionsDone++;
	if(connectionsStarted < connectionTargetCount) {
		connectionsStarted++;
		auth->connect(nullptr, CFG_GET("password")->getString(), Callback<Authentication::CallbackOnAuthResult>(nullptr, &onAuthResult));
	}
}

void onGameResult(IListener* instance, Authentication* auth, TS_ResultCode result, PacketSession * gameServerSocket) {
	fprintf(stderr, "login to GS result: %d\n", result);
	if(gameServerSocket) {
		connectionsDone++;
		gameServerSocket->getStream()->close();
		if(connectionsStarted < connectionTargetCount) {
			connectionsStarted++;
			auth->connect(nullptr, CFG_GET("password")->getString(), Callback<Authentication::CallbackOnAuthResult>(nullptr, &onAuthResult));
		}
	}
}

//conn bench
void onSocketStateChange(IListener* instance, Stream* socket, Stream::State oldState, Stream::State newState) {
	if(newState == Stream::ConnectedState) {
		socket->close();
//		if(delay == 0)
//			socket->close();
//		else
//			uv_timer_start(timers[reinterpret_cast<intptr_t>(instance)], &onSocketTimer, delay, 0);
	} else if(newState == Stream::UnconnectedState) {
		connectionsDone++;
		if(connectionsStarted < connectionTargetCount) {
			connectionsStarted++;
			socket->connect(ip, port);
		}
	}
}

void onSocketTimer(uv_timer_t* handle) {
	Stream* socket = (Stream*)handle->data;
	socket->close();
}
