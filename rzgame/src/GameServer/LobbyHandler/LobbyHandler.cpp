#include "LobbyHandler.h"
#include "../ClientSession.h"
#include "../../GlobalConfig.h"
#include "Core/CharsetConverter.h"
#include "../ReferenceData/ReferenceDataMgr.h"

#include "GameClient/TS_SC_CHARACTER_LIST.h"

namespace GameServer {

void LobbyHandler::onPacketReceived(const TS_MESSAGE *packet) {
	switch(packet->id) {
		case TS_CS_CHARACTER_LIST::packetID:
			onCharacterListQuery(static_cast<const TS_CS_CHARACTER_LIST*>(packet));
			break;

		case TS_CS_LOGIN::packetID:
			onCharacterLogin(static_cast<const TS_CS_LOGIN*>(packet));
			break;

		case TS_CS_CHECK_CHARACTER_NAME::packetID:
			onCheckCharacterName(static_cast<const TS_CS_CHECK_CHARACTER_NAME*>(packet));
			break;

		case TS_CS_CREATE_CHARACTER::packetID:
			packet->process(this, &LobbyHandler::onCreateCharacter, session->getVersion());
			break;

		case TS_CS_DELETE_CHARACTER::packetID:
			onDeleteCharacter(static_cast<const TS_CS_DELETE_CHARACTER*>(packet));
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

	auto& results = query->getResults();

	characterList.characters.reserve(results.size());

	auto it = results.begin();
	auto itEnd = results.end();
	for(; it != itEnd; ++it) {
		LOBBY_CHARACTER_INFO characterInfo;
		const CharacterLightBinding::Output* dbLine = it->get();

		characterInfo.sex = dbLine->sex;
		characterInfo.race = dbLine->race;
		memcpy(characterInfo.model_id, dbLine->model, sizeof(characterInfo.model_id));
		characterInfo.hair_color_index = dbLine->hair_color_index;
		characterInfo.hair_color_rgb = dbLine->hair_color_rgb;
		characterInfo.hide_equip_flag = dbLine->hide_equip_flag;
		characterInfo.texture_id = dbLine->texture_id;
		characterInfo.level = dbLine->lv;
		characterInfo.job = dbLine->job;
		characterInfo.job_level = dbLine->jlv;
		characterInfo.exp_percentage = dbLine->exp;
		characterInfo.hp = dbLine->hp;
		characterInfo.mp = dbLine->mp;
		characterInfo.permission = dbLine->permission;
		characterInfo.is_banned = false;
		characterInfo.name = dbLine->name;
		characterInfo.skin_color = dbLine->skin_color;
		sprintf(characterInfo.szCreateTime, "%04d-%02d-%02d %02d:%02d:%02d",
				dbLine->create_time.year,
				dbLine->create_time.month,
				dbLine->create_time.day,
				dbLine->create_time.hour,
				dbLine->create_time.minute,
				dbLine->create_time.second);
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

	characters = std::move(results);
}

void LobbyHandler::onCheckCharacterName(const TS_CS_CHECK_CHARACTER_NAME *packet) {
	std::string name = Utils::convertToString(packet->name, sizeof(packet->name));

	if(name.size() > 18 || name.size() < 4) { //TODO || isBanned(name)
		log(LL_Info, "Character name too long or too short: \"%s\"\n", name.c_str());
		session->sendResult(packet, TS_RESULT_INVALID_TEXT, 0);
	} else if(!checkTextAgainstEncoding(name)) {
		log(LL_Info, "Character name contains invalid characters for encoding %s: \"%s\"\n", CharsetConverter::getEncoding().c_str(), name.c_str());
		session->sendResult(packet, TS_RESULT_INVALID_TEXT, 0);
	} else if(ReferenceDataMgr::get()->isWordBanned(name)) {
		log(LL_Info, "Character name is banned: \"%s\"\n", name.c_str());
		session->sendResult(packet, TS_RESULT_INVALID_TEXT, 0);
	} else {
		CheckCharacterNameBinding::Input input;
		input.character_name = name;

		characterListQuery.executeDbQuery<CheckCharacterNameBinding>(this, &LobbyHandler::onCheckCharacterNameExistsResult, input);
	}
}

void LobbyHandler::onCheckCharacterNameExistsResult(DbQueryJob<CheckCharacterNameBinding> *query) {
	if(query->getResults().size() == 0) {
		session->sendResult(TS_CS_CHECK_CHARACTER_NAME::packetID, TS_RESULT_SUCCESS, 0);
		log(LL_Info, "Character name \"%s\" is free\n", query->getInput()->character_name.c_str());
	} else {
		session->sendResult(TS_CS_CHECK_CHARACTER_NAME::packetID, TS_RESULT_ALREADY_EXIST, 0);
		log(LL_Info, "Character name \"%s\" is already used\n", query->getInput()->character_name.c_str());
	}
}

void LobbyHandler::onCreateCharacter(const TS_CS_CREATE_CHARACTER *packet) {

}

void LobbyHandler::onDeleteCharacter(const TS_CS_DELETE_CHARACTER *packet) {

}

void LobbyHandler::onCharacterLogin(const TS_CS_LOGIN *packet) {
	std::unique_ptr<CharacterLight> selectCharacter;

	auto it = characters.begin();
	auto itEnd = characters.end();
	for(; it != itEnd; ++it) {
		std::unique_ptr<CharacterLight>& character = *it;
		if(packet->name == character->name) {
			selectCharacter = std::move(character);
			break;
		}
	}

	session->lobbyExitResult(std::move(selectCharacter));
}

bool LobbyHandler::checkTextAgainstEncoding(const std::string& text) {
	std::string checkEncoding = CharsetConverter::getEncoding() + "//IGNORE";
	CharsetConverter keepOnlyValid(checkEncoding.c_str(), checkEncoding.c_str());

	std::string convertedText;
	keepOnlyValid.convert(text, convertedText, 1.0f);

	return convertedText == text;
}

}
