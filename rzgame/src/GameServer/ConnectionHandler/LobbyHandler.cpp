#include "LobbyHandler.h"
#include "../ClientSession.h"
#include "../../GlobalConfig.h"

#include "GameClient/TS_SC_CHARACTER_LIST.h"
#include "GameClient/TS_SC_LOGIN_RESULT.h"

namespace GameServer {

void LobbyHandler::onPacketReceived(const TS_MESSAGE *packet) {
	switch(packet->id) {
		case TS_CS_CHARACTER_LIST::packetID:
			onCharacterListQuery(static_cast<const TS_CS_CHARACTER_LIST*>(packet));
			break;

		case TS_CS_LOGIN::packetID:
			onCharacterLogin(static_cast<const TS_CS_LOGIN*>(packet));
			break;
	}
}

void LobbyHandler::onCharacterListQuery(const TS_CS_CHARACTER_LIST*) {
	CharacterLightBinding::Input input;
	input.account_id = session->getAccountId();

	characterListQuery.executeDbQuery<CharacterLightBinding>(this, &LobbyHandler::onCharacterListResult, input);
}

void LobbyHandler::onCharacterListResult(DbQueryJob<CharacterLightBinding> *query) {
	TS_SC_CHARACTER_LIST characterList;

	auto results = query->getResults();

	characterList.characters.reserve(results.size());
	characters = results;

	auto it = results.begin();
	auto itEnd = results.end();
	for(; it != itEnd; ++it) {
		LOBBY_CHARACTER_INFO characterInfo;
		const CharacterLightBinding::Output& dbLine = *it;

		characterInfo.sex = dbLine.sex;
		characterInfo.race = dbLine.race;
		memcpy(characterInfo.model_id, dbLine.model, sizeof(characterInfo.model_id));
		characterInfo.hair_color_index = dbLine.hair_color_index;
		characterInfo.hair_color_rgb = dbLine.hair_color_rgb;
		characterInfo.hide_equip_flag = dbLine.hide_equip_flag;
		characterInfo.texture_id = dbLine.texture_id;
		characterInfo.level = dbLine.lv;
		characterInfo.job = dbLine.job;
		characterInfo.job_level = dbLine.jlv;
		characterInfo.exp_percentage = dbLine.exp;
		characterInfo.hp = dbLine.hp;
		characterInfo.mp = dbLine.mp;
		characterInfo.permission = dbLine.permission;
		characterInfo.is_banned = false;
		strncpy(characterInfo.name, dbLine.name, sizeof(characterInfo.name));
		characterInfo.skin_color = dbLine.skin_color;
		sprintf(characterInfo.szCreateTime, "%04d-%02d-%02d %02d:%02d:%02d",
				dbLine.create_time.year,
				dbLine.create_time.month,
				dbLine.create_time.day,
				dbLine.create_time.hour,
				dbLine.create_time.minute,
				dbLine.create_time.second);
		strcpy(characterInfo.szDeleteTime, "9999-12-31 00:00:00");

		memset(&characterInfo.wear_info, 0, sizeof(characterInfo.wear_info));
		memset(&characterInfo.wear_info_2, 0, sizeof(characterInfo.wear_info_2));
		memset(&characterInfo.wear_item_enhance_info, 0, sizeof(characterInfo.wear_item_enhance_info));
		memset(&characterInfo.wear_item_enhance_info_2, 0, sizeof(characterInfo.wear_item_enhance_info_2));
		memset(&characterInfo.wear_item_level_info, 0, sizeof(characterInfo.wear_item_level_info));
		memset(&characterInfo.wear_item_level_info_2, 0, sizeof(characterInfo.wear_item_level_info_2));
		memset(&characterInfo.wear_item_elemental_type, 0, sizeof(characterInfo.wear_item_elemental_type));
		memset(&characterInfo.wear_appearance_code, 0, sizeof(characterInfo.wear_appearance_code));

		characterList.characters.push_back(characterInfo);
	}

	characterList.current_server_time = 0;
	characterList.last_character_idx = 0;

	session->sendPacket(characterList, CONFIG_GET()->game.clients.epic.get());

	log(LL_Debug, "Account %s has %d characters\n", session->getAccount().c_str(), (int)characterList.characters.size());
}

void LobbyHandler::onCharacterLogin(const TS_CS_LOGIN *packet) {
	std::string charName = Utils::convertToString(packet->szName, sizeof(packet->szName) - 1);
	const CharacterLight* selectCharacter = nullptr;

	auto it = characters.begin();
	auto itEnd = characters.end();
	for(; it != itEnd; ++it) {
		const CharacterLight& character = *it;
		if(charName == Utils::convertToString(character.name, sizeof(character.name)-1)) {
			selectCharacter = &character;
			break;
		}
	}

	if(selectCharacter == nullptr) {
		TS_SC_LOGIN_RESULT loginResult = {0};
		loginResult.result = TS_RESULT_NOT_EXIST;
		session->sendPacket(loginResult, session->getVersion());
		session->abortSession();
	} else {
		//session->initCharacter(*selectCharacter);
	}
}

}
