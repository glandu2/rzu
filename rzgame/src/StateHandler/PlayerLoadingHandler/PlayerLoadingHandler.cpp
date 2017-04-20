#include "PlayerLoadingHandler.h"
#include "ClientSession.h"
#include "Config/GlobalConfig.h"
#include "Component/Character/Character.h"
#include "Component/Inventory/Item.h"

#include "GameClient/TS_SC_LOGIN_RESULT.h"
#include "GameClient/TS_SC_URL_LIST.h"
#include "GameClient/TS_SC_STAT_INFO.h"
#include "GameClient/TS_SC_PROPERTY.h"

namespace GameServer {

PlayerLoadingHandler::PlayerLoadingHandler(ClientSession *session, game_sid_t sid)
	: ConnectionHandler(session)
{
	DB_CharacterBinding::Input input;
	input.sid = sid;
	characterQuery.executeDbQuery<DB_CharacterBinding>(this, &PlayerLoadingHandler::onCharacterResult, input);

	TS_SC_URL_LIST urlListPacket;
	urlListPacket.url_list = CONFIG_GET()->game.urlList.get();
	session->sendPacket(urlListPacket);
}

void PlayerLoadingHandler::onPacketReceived(const TS_MESSAGE *packet) {

}

void PlayerLoadingHandler::onCharacterResult(DbQueryJob<DB_CharacterBinding> *query) {
	auto& results = query->getResults();
	TS_SC_LOGIN_RESULT loginResult = {0};

	if(!results.empty()) {
		character.reset(new Character(session, query->getInput()->sid, session->getAccount(), results[0].get()));

		loginResult.result = (session->getVersion() >= EPIC_7_1) ? TS_RESULT_SUCCESS : true;
		loginResult.handle = character->handle;
		loginResult.x = character->x;
		loginResult.y = character->y;
		loginResult.z = character->z;
		loginResult.layer = character->layer;
		loginResult.face_direction = character->face_direction;
		loginResult.region_size = 180;
		loginResult.hp = character->hp;
		loginResult.mp = character->mp;
		loginResult.max_hp = character->maxHp;
		loginResult.max_mp = character->maxMp;
		loginResult.havoc = 0;
		loginResult.max_havoc = 1000;
		loginResult.sex = character->sex;
		loginResult.race = character->race;
		loginResult.skin_color = character->skinColor;
		loginResult.faceId = character->baseModel.faceId;
		loginResult.hairId = character->baseModel.hairId;
		loginResult.name = character->name;
		loginResult.cell_size = 6;
		loginResult.guild_id = 0;
		session->sendPacket(loginResult);

		DB_ItemBinding::Input input;
		input.owner_id = character->sid;
		characterQuery.executeDbQuery<DB_ItemBinding>(this, &PlayerLoadingHandler::onItemListResult, input);
	} else {
		loginResult.result = TS_RESULT_DB_ERROR;
		session->sendPacket(loginResult);
		session->playerLoadingResult(TS_RESULT_DB_ERROR, std::unique_ptr<Character>());
	}
}

void PlayerLoadingHandler::onItemListResult(DbQueryJob<DB_ItemBinding> *query) {
	std::vector<std::unique_ptr<DB_Item>>& results = query->getResults();

	Inventory& inventory = character->inventory;
	inventory.initializeItems(results);

	character->synchronizeWithClient();

	session->playerLoadingResult(TS_RESULT_SUCCESS, std::move(character));
}

} // namespace GameServer
