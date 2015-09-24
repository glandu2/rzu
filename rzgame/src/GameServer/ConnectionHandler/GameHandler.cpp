#include "GameHandler.h"
#include "GameClient/TS_SC_LOGIN_RESULT.h"
#include "../ClientSession.h"

namespace GameServer {

GameHandler::GameHandler(ClientSession *session, const CharacterLight &characterLight)
	: ConnectionHandler(session)
{
	characterData = characterLight;

	CharacterDetailsBinding::Input input;
	input.sid = characterLight.sid;
	characterQuery.executeDbQuery<CharacterDetailsBinding>(this, &GameHandler::onCharacterResult, input);
}

void GameHandler::onPacketReceived(const TS_MESSAGE *packet) {

}

void GameHandler::onCharacterResult(DbQueryJob<CharacterDetailsBinding> *query) {
	auto results = query->getResults();

	if(!results.empty()) {
		characterDetails = query->getResults().front();
	} else {
		TS_SC_LOGIN_RESULT loginResult = {0};
		loginResult.result = TS_RESULT_DB_ERROR;
		session->sendPacket(loginResult, session->getVersion());
		session->abortSession();
	}
}

} // namespace GameServer
