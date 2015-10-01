#include "PlayerLoadingHandler.h"
#include "GameClient/TS_SC_LOGIN_RESULT.h"
#include "../ClientSession.h"

namespace GameServer {

PlayerLoadingHandler::PlayerLoadingHandler(ClientSession *session, std::unique_ptr<CharacterLight> characterLight)
	: ConnectionHandler(session)
{
	CharacterDetailsBinding::Input input;
	input.sid = characterData->sid;
	characterData = std::move(characterLight);
	characterQuery.executeDbQuery<CharacterDetailsBinding>(this, &PlayerLoadingHandler::onCharacterResult, input);
}

void PlayerLoadingHandler::onPacketReceived(const TS_MESSAGE *packet) {

}

void PlayerLoadingHandler::onCharacterResult(DbQueryJob<CharacterDetailsBinding> *query) {
	auto& results = query->getResults();

	if(!results.empty()) {
		characterDetails = std::move(query->getResults().front());
		session->playerLoadingResult(TS_RESULT_SUCCESS);
	} else {
		session->playerLoadingResult(TS_RESULT_DB_ERROR);
	}
}

} // namespace GameServer
