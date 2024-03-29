#pragma once

#include "ConfigInfo.h"
#include "Core/Utils.h"

#define CONFIG_FILE_KEY "configfile"

struct ListenerConfig {
	cval<std::string>& listenIp;
	cval<int>&port, &idleTimeout;
	cval<bool>& autoStart;

	ListenerConfig(
	    const std::string& prefix, const char* defaultIp, int defaultPort, bool autoStart = true, int idleTimeout = 0)
	    : listenIp(CFG_CREATE(prefix + ".ip", defaultIp)),
	      port(CFG_CREATE(prefix + ".port", defaultPort)),
	      idleTimeout(CFG_CREATE(prefix + ".idletimeout", idleTimeout)),
	      autoStart(CFG_CREATE(prefix + ".autostart", autoStart)) {}
};

struct RZU_EXTERN GlobalCoreConfig : public IListener {
	struct App : public IListener {
		cval<std::string>&appName, &configfile;
		cval<bool>& useTcpNoDelay;
		cval<bool>& showHiddenConfig;
		cval<std::string>& encoding;
		cval<std::string>& streamCipher;

		App()
		    : appName(CFG_CREATE("core.appname", Utils::getApplicationName())),
		      configfile(CFG_CREATE(CONFIG_FILE_KEY, std::string(Utils::getApplicationName()) + ".opt")),
		      useTcpNoDelay(CFG_CREATE("core.usetcpnodelay", false)),
		      showHiddenConfig(CFG_CREATE("core.config.showhidden", false)),
		      encoding(CFG_CREATE("core.encoding", "CP1252")),
		      streamCipher(CFG_CREATE("core.stream_cipher", "}h79q~B%al;k'y $E", true)) {
			Utils::autoSetAbsoluteDir(configfile);
			streamCipher.addListener(this, &updateStreamCipher);
			updateStreamCipher(this);
		}
		static void updateStreamCipher(IListener* instance);
	} app;

	struct Client {
		cval<std::string>& authVersion;
		cval<std::string>& gameVersion;
		cval<std::string>& boraCodeGeneratorCommand;

		Client()
		    : authVersion(CFG_CREATE("client.auth_version", "0")),
		      gameVersion(CFG_CREATE("client.game_version", "0")),
		      boraCodeGeneratorCommand(
		          CFG_CREATE("client.bora_command",
		                     "https://accounts.play2bit.com/member/oauth/"
		                     "authorize?response_type=code&client_id=rAu3fzNDKd1QgEsL&redirect_uri=https://"
		                     "accounts.play2bit.com/member/oauth/pub")) {
			updateDefaultVersions();
		}

		void updateDefaultVersions();
	} client;

	struct AdminConfig {
		ListenerConfig listener;
		cval<int>& dumpMode;

		AdminConfig()
		    : listener("admin.console", "127.0.0.1", 4501, true, 0),
		      dumpMode(CFG_CREATE("admin.dump_mode", 0))  // 1: no dump, anything else: create dump on crash
		{}
	} admin;

	struct Log : public IListener {
		cval<bool>& enable;
		cval<std::string>&dir, &file, &level, &consoleLevel;
		cval<int>& maxQueueSize;
		cval<bool>& dumpPacketErrors;

		Log()
		    : enable(CFG_CREATE("core.log.enable", true)),
		      dir(CFG_CREATE("core.log.dir", "log")),
		      file(CFG_CREATE("core.log.file", CFG_GET("core.appname")->getString() + ".log")),
		      level(CFG_CREATE("core.log.level", "info")),
		      consoleLevel(CFG_CREATE("core.log.consolelevel", "info")),
		      maxQueueSize(CFG_CREATE("core.log.maxqueuesize", 10000)),
		      dumpPacketErrors(CFG_CREATE("core.log.dump_packet_errors", false)) {
			Utils::autoSetAbsoluteDir(dir);
			level.addListener(this, &updateConsoleLevel);
		}

		static void updateConsoleLevel(IListener* instance);
	} log;

	struct PacketLog {
		cval<bool>& dumpRaw;
		cval<bool>& dumpJson;

		PacketLog()
		    : dumpRaw(CFG_CREATE("trafficdump.dump_raw", false)), dumpJson(CFG_CREATE("trafficdump.dump_json", true)) {}
	} packetLog;

	struct Statistics {
		cstatval<int>&connectionCount, &disconnectionCount;

		Statistics()
		    : connectionCount(CFG_STAT_CREATE("stats.connections", 0)),
		      disconnectionCount(CFG_STAT_CREATE("stats.disconnections", 0)) {}
	} stats;

	GlobalCoreConfig() { app.appName.addListener(this, &updateOtherFile); }

	static GlobalCoreConfig* get();
	static void init();
	static void updateOtherFile(IListener* instance);
};
