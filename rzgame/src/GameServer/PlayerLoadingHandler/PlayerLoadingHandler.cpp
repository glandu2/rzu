#include "PlayerLoadingHandler.h"
#include "GameClient/TS_SC_LOGIN_RESULT.h"
#include "../ClientSession.h"
#include "GameClient/TS_SC_URL_LIST.h"
#include "../../GlobalConfig.h"

namespace GameServer {

PlayerLoadingHandler::PlayerLoadingHandler(ClientSession *session, uint64_t sid)
	: ConnectionHandler(session)
{
	CharacterBinding::Input input;
	input.sid = sid;
	characterQuery.executeDbQuery<CharacterBinding>(this, &PlayerLoadingHandler::onCharacterResult, input);

	TS_SC_URL_LIST urlListPacket;
	urlListPacket.url_list = CONFIG_GET()->game.urlList.get();
	session->sendPacket(urlListPacket, session->getVersion());
}

void PlayerLoadingHandler::onPacketReceived(const TS_MESSAGE *packet) {

}

void PlayerLoadingHandler::onCharacterResult(DbQueryJob<CharacterBinding> *query) {
	auto& results = query->getResults();

	if(!results.empty()) {
		characterData = std::move(query->getResults().front());
		session->playerLoadingResult(TS_RESULT_SUCCESS);
	} else {
		session->playerLoadingResult(TS_RESULT_DB_ERROR);
	}
}

} // namespace GameServer
