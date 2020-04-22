#pragma once

#include "Config/ConfigInfo.h"

namespace UploadServer {

struct ConnectionConfig {
	cval<std::string>& ip;
	cval<int>& port;

	ConnectionConfig(std::string prefix, int defaultPort, const char* defaultIp = "127.0.0.1")
	    : ip(CFG_CREATE(prefix + ".ip", defaultIp)), port(CFG_CREATE(prefix + ".port", defaultPort)) {}
};

struct GlobalConfig {
	ConnectionConfig upload;
	ConnectionConfig game;

	GlobalConfig() : upload("upload.clients", 4617), game("upload.game", 4616) {}

	static GlobalConfig* get();
	static void init();
};

auto inline CONFIG_GET() {
	return UploadServer::GlobalConfig::get();
}

}  // namespace UploadServer
