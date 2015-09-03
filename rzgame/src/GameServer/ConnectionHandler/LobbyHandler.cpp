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
	Database::CharacterList::Input input;
	input.account_id = session->getAccountId();

	characterListQuery.executeDbQuery<Database::CharacterList>(this, &LobbyHandler::onCharacterListResult, input);
}

void LobbyHandler::onCharacterListResult(DbQueryJob<Database::CharacterList> *query) {
	TS_SC_CHARACTER_LIST characterList;

	auto results = query->getResults();

	characterList.characters.reserve(results.size());
	characters.clear();
	characters.reserve(results.size());

	auto it = results.begin();
	auto itEnd = results.end();
	for(; it != itEnd; ++it) {
		LOBBY_CHARACTER_INFO characterInfo;
		const Database::CharacterList::Output& dbLine = *it;

		characterInfo.sex = dbLine.sex;
		characterInfo.race = dbLine.race;
		characterInfo.model_id[0] = dbLine.model_00;
		characterInfo.model_id[1] = dbLine.model_01;
		characterInfo.model_id[2] = dbLine.model_02;
		characterInfo.model_id[3] = dbLine.model_03;
		characterInfo.model_id[4] = dbLine.model_04;
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

		CharacterId characterId = { dbLine.sid, std::string(dbLine.name) };
		characters.push_back(characterId);
	}

	characterList.current_server_time = 0;
	characterList.last_character_idx = 0;

	session->sendPacket(characterList, CONFIG_GET()->game.clients.epic.get());

	debug("Account %s has %d characters\n", session->getAccount().c_str(), characterList.characters.size());
}

void LobbyHandler::onCharacterLogin(const TS_CS_LOGIN *packet) {
	std::string charName = Utils::convertToString(packet->szName, sizeof(packet->szName) - 1);
	uint32_t sid = UINT32_MAX;

	auto it = characters.begin();
	auto itEnd = characters.end();
	for(; it != itEnd; ++it) {
		const CharacterId& characterId = *it;
		if(characterId.name == charName) {
			sid = characterId.sid;
			break;
		}
	}

	//if(sid == UINT32_MAX) {
		TS_SC_LOGIN_RESULT loginResult = {0};
		loginResult.result = TS_RESULT_NOT_EXIST;
		session->sendPacket(loginResult, session->getVersion());
	//}
}

}
