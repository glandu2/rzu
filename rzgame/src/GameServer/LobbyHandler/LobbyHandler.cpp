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
	auto& results = query->getResults();
	characters = std::move(results);

	CharacterWearInfoBinding::Input input;
	input.account_id = session->getAccountId();
	characterListQuery.executeDbQuery<CharacterWearInfoBinding>(this, &LobbyHandler::onCharacterWearInfoResult, input);
}

void LobbyHandler::onCharacterWearInfoResult(DbQueryJob<CharacterWearInfoBinding>* query) {
	TS_SC_CHARACTER_LIST characterList;
	characterList.characters.reserve(characters.size());

	auto& wearInfoResults = query->getResults();

	auto it = characters.begin();
	auto itEnd = characters.end();
	for(; it != itEnd; ++it) {
		LOBBY_CHARACTER_INFO characterInfo = {0};
		const CharacterLight* dbCharacterInfo = it->get();

		characterInfo.sex = dbCharacterInfo->sex;
		characterInfo.race = dbCharacterInfo->race;
		memcpy(characterInfo.model_id, dbCharacterInfo->model, sizeof(characterInfo.model_id));
		characterInfo.hair_color_index = dbCharacterInfo->hair_color_index;
		characterInfo.hair_color_rgb = dbCharacterInfo->hair_color_rgb;
		characterInfo.hide_equip_flag = dbCharacterInfo->hide_equip_flag;
		characterInfo.texture_id = dbCharacterInfo->texture_id;
		characterInfo.level = dbCharacterInfo->lv;
		characterInfo.job = dbCharacterInfo->job;
		characterInfo.job_level = dbCharacterInfo->jlv;
		characterInfo.exp_percentage = dbCharacterInfo->exp;
		characterInfo.hp = dbCharacterInfo->hp;
		characterInfo.mp = dbCharacterInfo->mp;
		characterInfo.permission = dbCharacterInfo->permission;
		characterInfo.is_banned = false;
		characterInfo.name = dbCharacterInfo->name;
		characterInfo.skin_color = dbCharacterInfo->skin_color;
		sprintf(characterInfo.szCreateTime, "%04d-%02d-%02d %02d:%02d:%02d",
				dbCharacterInfo->create_time.year,
				dbCharacterInfo->create_time.month,
				dbCharacterInfo->create_time.day,
				dbCharacterInfo->create_time.hour,
				dbCharacterInfo->create_time.minute,
				dbCharacterInfo->create_time.second);
		strcpy(characterInfo.szDeleteTime, "9999-12-31 00:00:00");


		auto it = wearInfoResults.begin();
		auto itEnd = wearInfoResults.end();
		for(; it != itEnd; ++it) {
			const CharacterWearInfo* dbWearInfo = it->get();

			if(dbCharacterInfo->sid == dbWearInfo->character_sid && dbWearInfo->wear_info < sizeof(characterInfo.wear_info) / sizeof(characterInfo.wear_info[0])) {
				characterInfo.wear_info[dbWearInfo->wear_info] = dbWearInfo->code;
				characterInfo.wear_item_level_info[dbWearInfo->wear_info] = dbWearInfo->level;
				characterInfo.wear_item_enhance_info[dbWearInfo->wear_info] = dbWearInfo->enhance;
				characterInfo.wear_item_elemental_type[dbWearInfo->wear_info] = dbWearInfo->elemental_effect_type;
				characterInfo.wear_appearance_code[dbWearInfo->wear_info] = dbWearInfo->appearance_code;
			}
		}

		characterList.characters.push_back(characterInfo);
	}

	characterList.current_server_time = 0;
	characterList.last_character_idx = 0;

	session->sendPacket(characterList, CONFIG_GET()->game.clients.epic.get());

	log(LL_Debug, "Account %s has %d characters\n", session->getAccount().c_str(), (int)characterList.characters.size());

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
