#define __STDC_LIMIT_MACROS
#include "ClientSession.h"
#include <string.h>
#include "AuthServerSession.h"
#include "../GlobalConfig.h"
#include "DbQueryJobCallback.h"

#include "Packets/PacketEnums.h"
#include "Packets/TS_SC_RESULT.h"
#include "Packets/TS_GA_CLIENT_LOGIN.h"
#include "Packets/TS_SC_CHARACTER_LIST.h"

namespace GameServer {

void ClientSession::init() {
}

void ClientSession::deinit() {
}

ClientSession::ClientSession() {
}

ClientSession::~ClientSession() {
	info("Account %s disconnected\n", account.c_str());
}

void ClientSession::onAccountLoginResult(uint16_t result, std::string account, uint32_t accountId, char nPCBangUser, uint32_t nEventCode, uint32_t nAge, uint32_t nContinuousPlayTime, uint32_t nContinuousLogoutTime) {
	TS_SC_RESULT loginResult;
	TS_MESSAGE::initMessage(&loginResult);

	loginResult.request_msg_id = TS_CS_ACCOUNT_WITH_AUTH::packetID;
	loginResult.result = result;
	loginResult.value = 0;
	sendPacket(&loginResult);

	if(result != TS_RESULT_SUCCESS) {
		warn("Login failed for account %s: %d\n", account.c_str(), result);
	} else {
		debug("Login success for account %s\n", account.c_str());
		this->accountId = accountId;
		this->account = account;
	}
}

void ClientSession::onCharacterList(DbQueryJob<Database::CharacterList> *query) {
	TS_SC_CHARACTER_LIST characterList;

	auto results = query->getResults();

	characterList.characters.reserve(results.size());

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
	}

	characterList.current_server_time = 0;
	characterList.last_character_idx = 1;

	sendPacket(characterList, CONFIG_GET()->game.clients.epic.get());

	debug("Account %s has %d characters\n", account.c_str(), characterList.characters.size());
}

void ClientSession::onPacketReceived(const TS_MESSAGE* packet) {
	switch(packet->id) {
		case TS_CS_ACCOUNT_WITH_AUTH::packetID: {
			MessageBuffer buffer(packet, CONFIG_GET()->game.clients.epic.get());
			process<TS_CS_ACCOUNT_WITH_AUTH>(&buffer, &ClientSession::onAccountWithAuth);
			break;
		}

		case TS_CS_CHARACTER_LIST::packetID:
			onCharacterListQuery(static_cast<const TS_CS_CHARACTER_LIST*>(packet));
			break;
	}
}

void ClientSession::onAccountWithAuth(TS_CS_ACCOUNT_WITH_AUTH& packet) {
	std::string account = Utils::convertToString(packet.account, sizeof(packet.account) - 1);

	AuthServerSession::get()->loginClient(this, account, packet.one_time_key);
}

void ClientSession::onCharacterListQuery(const TS_CS_CHARACTER_LIST* packet) {
	Database::CharacterList::Input input;
	input.account_id = accountId;

	characterListQuery.executeDbQuery<Database::CharacterList>(this, &ClientSession::onCharacterList, input);
}

} //namespace UploadServer
