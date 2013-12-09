#include "ClientInfo.h"
#include "ServerInfo.h"
#include "uv.h"

int main() {
	ClientInfo::startServer();
	ServerInfo::startServer();

	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}
