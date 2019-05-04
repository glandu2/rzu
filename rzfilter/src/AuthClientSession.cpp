#include "AuthClientSession.h"
#include "AuthClient/TS_AC_SERVER_LIST.h"
#include "AuthClient/TS_CA_SELECT_SERVER.h"
#include "GameClientSessionManager.h"
#include "GlobalConfig.h"
#include <algorithm>
#include <vector>

AuthClientSession::AuthClientSession(InputParameters parameters)
    : ClientSession(
          true, parameters.gameClientSessionManager, parameters.filterManager, parameters.converterFilterManager) {}

AuthClientSession::~AuthClientSession() {}

std::string AuthClientSession::getServerIp() {
	return CONFIG_GET()->server.ip.get();
}

uint16_t AuthClientSession::getServerPort() {
	return CONFIG_GET()->server.port.get();
}
