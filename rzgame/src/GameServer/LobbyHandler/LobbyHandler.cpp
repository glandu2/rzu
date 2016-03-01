#include "LobbyHandler.h"
#include "../ClientSession.h"
#include "../../GlobalConfig.h"
#include "Core/CharsetConverter.h"
#include "Core/PrintfFormats.h"
#include "../ReferenceData/ReferenceDataMgr.h"

#include "../Model/Character.h"

#include "GameClient/TS_SC_CHARACTER_LIST.h"

namespace GameServer {

LobbyHandler::LobbyHandler(ClientSession *session) : ConnectionHandler(session), charactersPopulated(false) {}

void LobbyHandler::onPacketReceived(const TS_MESSAGE *packet) {
	switch(packet->id) {
		case TS_CS_CHARACTER_LIST::packetID:
			onCharacterListQuery(static_cast<const TS_CS_CHARACTER_LIST*>(packet));
			break;

		case_packet_is(TS_CS_LOGIN)
			packet->process(this, &LobbyHandler::onCharacterLogin, session->getVersion());
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

	lobbyQueries.executeDbQuery<CharacterLightBinding>(this, &LobbyHandler::onCharacterListResult, input);
}
void LobbyHandler::onCharacterListResult(DbQueryJob<CharacterLightBinding> *query) {
	auto& results = query->getResults();
	characters = std::move(results);
	charactersPopulated = true;

	CharacterWearInfoBinding::Input input;
	input.account_id = session->getAccountId();
	lobbyQueries.executeDbQuery<CharacterWearInfoBinding>(this, &LobbyHandler::onCharacterWearInfoResult, input);
}

void LobbyHandler::onCharacterWearInfoResult(DbQueryJob<CharacterWearInfoBinding>* query) {
	TS_SC_CHARACTER_LIST characterList;
	characterList.characters.reserve(characters.size());

	auto& wearInfoResults = query->getResults();

	characterList.current_server_time = 0;
	characterList.last_character_idx = 0;
	char lastCharacterLoginTime[30] = {0};

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
		characterInfo.exp_percentage = 0;
		characterInfo.hp = dbCharacterInfo->hp;
		characterInfo.mp = dbCharacterInfo->mp;
		characterInfo.permission = dbCharacterInfo->permission;
		characterInfo.is_banned = false;
		characterInfo.name = dbCharacterInfo->name;
		characterInfo.skin_color = dbCharacterInfo->skin_color;
		/*sprintf(characterInfo.szCreateTime, "%04d-%02d-%02d %02d:%02d:%02d",
				dbCharacterInfo->create_time.year,
				dbCharacterInfo->create_time.month,
				dbCharacterInfo->create_time.day,
				dbCharacterInfo->create_time.hour,
				dbCharacterInfo->create_time.minute,
				dbCharacterInfo->create_time.second);*/
		strcpy(characterInfo.szCreateTime, dbCharacterInfo->create_time);
		strcpy(characterInfo.szDeleteTime, "9999-12-31 00:00:00");

		auto wearIt = wearInfoResults.begin();
		auto wearItEnd = wearInfoResults.end();
		for(; wearIt != wearItEnd; ++wearIt) {
			const CharacterWearInfo* dbWearInfo = wearIt->get();

			if(dbCharacterInfo->sid == dbWearInfo->character_sid && dbWearInfo->wear_info >= 0 && (size_t)dbWearInfo->wear_info < sizeof(characterInfo.wear_info) / sizeof(characterInfo.wear_info[0])) {
				characterInfo.wear_info[dbWearInfo->wear_info] = dbWearInfo->code;
				characterInfo.wear_item_level_info[dbWearInfo->wear_info] = dbWearInfo->level;
				characterInfo.wear_item_enhance_info[dbWearInfo->wear_info] = dbWearInfo->enhance;
				characterInfo.wear_item_elemental_type[dbWearInfo->wear_info] = dbWearInfo->elemental_effect_type;
				characterInfo.wear_appearance_code[dbWearInfo->wear_info] = dbWearInfo->appearance_code;
			}
		}

		if(strcmp(lastCharacterLoginTime, dbCharacterInfo->login_time) < 0) {
			characterList.last_character_idx = (uint16_t)characterList.characters.size();
			strcpy(lastCharacterLoginTime, dbCharacterInfo->login_time);
		}

		characterList.characters.push_back(characterInfo);
	}

	session->sendPacket(characterList);

	log(LL_Debug, "Account %s has %d characters\n", session->getAccount().c_str(), (int)characterList.characters.size());

}

void LobbyHandler::onCheckCharacterName(const TS_CS_CHECK_CHARACTER_NAME *packet) {
	std::string name = Utils::convertToString(packet->name, sizeof(packet->name));

	lastValidatedCharacterName.clear();

	if(name.size() > 18 || name.size() < 4) {
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

		lobbyQueries.executeDbQuery<CheckCharacterNameBinding>(this, &LobbyHandler::onCheckCharacterNameExistsResult, input);
	}
}

void LobbyHandler::onCheckCharacterNameExistsResult(DbQueryJob<CheckCharacterNameBinding> *query) {
	if(query->getResults().size() == 0) {
		session->sendResult(TS_CS_CHECK_CHARACTER_NAME::packetID, TS_RESULT_SUCCESS, 0);
		lastValidatedCharacterName = query->getInput()->character_name;
		log(LL_Debug, "Character name \"%s\" is free\n", query->getInput()->character_name.c_str());
	} else {
		session->sendResult(TS_CS_CHECK_CHARACTER_NAME::packetID, TS_RESULT_ALREADY_EXIST, 0);
		log(LL_Info, "Character name \"%s\" is already used\n", query->getInput()->character_name.c_str());
	}
}

void LobbyHandler::onCreateCharacter(const TS_CS_CREATE_CHARACTER *packet) {
	CreateCharacterBinding::Input input;

	if(packet->character.name != lastValidatedCharacterName) {
		log(LL_Warning, "Client tried to create character without prior check for name: %s\n", packet->character.name.c_str());
		session->sendResult(packet->packetID, TS_RESULT_INVALID_TEXT, 0);
	} else if(lobbyQueries.inProgress()) {
		log(LL_Warning, "Account %s sent create character with SQL query in progress\n", session->getAccount().c_str());
		session->sendResult(packet->packetID, TS_RESULT_NOT_ACTABLE, 0);
	} else if(charactersPopulated == false) {
		log(LL_Info, "Account %s sent create character without having retrieved character list\n", session->getAccount().c_str());
		session->sendResult(packet->packetID, TS_RESULT_NOT_ACTABLE, 0);
	} else if(characters.size() >= 6) {
		log(LL_Info, "Account %s sent create character without free slot\n", session->getAccount().c_str());
		session->sendResult(packet->packetID, TS_RESULT_LIMIT_MAX, 0);
	} else {
		lastValidatedCharacterName.clear();

		input.name = packet->character.name;
		strncpy(input.account_name, session->getAccount().c_str(), 60);
		input.account_name[60] = 0;
		input.account_id = session->getAccountId();
		input.slot = (uint32_t)characters.size();
		input.x = 19500;
		input.y = 51000;
		input.z = 0;
		input.layer = 0;
		input.race = packet->character.race;
		input.sex = packet->character.sex;
		input.level = 0;
		input.max_reached_level = 0;
		input.hp = 0;
		input.mp = 0;
		input.jlv = 0;
		input.jp = 0;
		input.cha = 0;
		input.skin_color = packet->character.skin_color;
		memcpy(&input.model, packet->character.model_id, sizeof(input.model));
		input.hair_color_index = packet->character.hair_color_index;
		input.texture_id = packet->character.texture_id;

		input.default_weapon_sid = ReferenceDataMgr::get()->allocateItemSid();
		input.default_armor_sid = ReferenceDataMgr::get()->allocateItemSid();
		input.default_bag_sid = ReferenceDataMgr::get()->allocateItemSid();

		bool useSecondaryStuff = packet->character.wear_info[2] == 602;
		switch(packet->character.race) {
			case 3:
				input.default_armor_code = useSecondaryStuff ? 240109 : 240100;
				input.default_weapon_code = 112100;
				break;
			case 5:
				input.default_armor_code = useSecondaryStuff ? 230109 : 230100;
				input.default_weapon_code = 103100;
				break;
			default:
				input.default_armor_code = useSecondaryStuff ? 220109 : 220100;
				input.default_weapon_code = 106100;
				break;

		}

		input.default_bag_code = 490001;

		lobbyQueries.executeDbQuery<CreateCharacterBinding>(this, &LobbyHandler::onCreateCharacterResult, input);
	}
}

void LobbyHandler::onCreateCharacterResult(DbQueryJob<CreateCharacterBinding> *query) {
	if(query->getInput()->out_character_sid == 0) {
		log(LL_Warning, "Create character \"%s\" failed for account %s\n", query->getInput()->name.c_str(), query->getInput()->account_name);
		session->sendResult(TS_CS_CREATE_CHARACTER::packetID, TS_RESULT_DB_ERROR, 0);
	} else {
		log(LL_Debug, "Created character \"%s\" for account %s with sid %" PRId64 "\n", query->getInput()->name.c_str(), query->getInput()->account_name, query->getInput()->out_character_sid);
		session->sendResult(TS_CS_CREATE_CHARACTER::packetID, TS_RESULT_SUCCESS, 0);
	}
}

void LobbyHandler::onDeleteCharacter(const TS_CS_DELETE_CHARACTER *packet) {
	std::string name = Utils::convertToString(packet->name, sizeof(packet->name) - 1);
	bool hasCharacter = false;

	auto it = characters.begin();
	auto itEnd = characters.end();
	for(; it != itEnd; ++it) {
		std::unique_ptr<CharacterLight>& character = *it;
		if(packet->name == character->name) {
			hasCharacter = true;
			break;
		}
	}

	if(!hasCharacter) {
		session->sendResult(TS_CS_DELETE_CHARACTER::packetID, TS_RESULT_NOT_EXIST, 0);
		log(LL_Info, "Account %s tried to delete non existing character %s\n", session->getAccount().c_str(), name.c_str());
	} else {
		DeleteCharacterBinding::Input input;
		input.character_name = name;

		lobbyQueries.executeDbQuery<DeleteCharacterBinding>(this, &LobbyHandler::onDeleteCharacterResult, input);
	}
}

void LobbyHandler::onDeleteCharacterResult(DbQueryJob<DeleteCharacterBinding> *query) {
	session->sendResult(TS_CS_DELETE_CHARACTER::packetID, TS_RESULT_SUCCESS, 0);
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

	if(Character::getObjectBySid(selectCharacter.get()->sid) == nullptr) {
		session->lobbyExitResult(TS_RESULT_SUCCESS, std::move(selectCharacter));
	} else {
		log(LL_Warning, "Character %s of account %s is already logged in\n", selectCharacter.get()->name.c_str(), session->getAccount().c_str());
		session->lobbyExitResult(TS_RESULT_ALREADY_EXIST, std::unique_ptr<CharacterLight>());
	}
}

bool LobbyHandler::checkTextAgainstEncoding(const std::string& text) {
	std::string checkEncoding = CharsetConverter::getEncoding() + "//IGNORE";
	CharsetConverter keepOnlyValid(checkEncoding.c_str(), checkEncoding.c_str());

	std::string convertedText;
	keepOnlyValid.convert(text, convertedText, 1.0f);

	return convertedText == text;
}

}
