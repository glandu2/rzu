#ifndef PACKETTEMPLATES_H
#define PACKETTEMPLATES_H

#include "AuthClient/TS_AC_ACCOUNT_NAME.h"
#include "AuthClient/TS_AC_AES_KEY_IV.h"
#include "AuthClient/TS_AC_RESULT.h"
#include "AuthClient/TS_AC_RESULT_WITH_STRING.h"
#include "AuthClient/TS_AC_SELECT_SERVER.h"
#include "AuthClient/TS_AC_SERVER_LIST.h"
#include "AuthClient/TS_AC_UPDATE_PENDING_TIME.h"
#include "AuthClient/TS_CA_ACCOUNT.h"
#include "AuthClient/TS_CA_DISTRIBUTION_INFO.h"
#include "AuthClient/TS_CA_IMBC_ACCOUNT.h"
#include "AuthClient/TS_CA_OTP_ACCOUNT.h"
#include "AuthClient/TS_CA_RSA_PUBLIC_KEY.h"
#include "AuthClient/TS_CA_SELECT_SERVER.h"
#include "AuthClient/TS_CA_SERVER_LIST.h"
#include "AuthClient/TS_CA_VERSION.h"

#include "GameClient/TS_CS_ACCOUNT_WITH_AUTH.h"
#include "GameClient/TS_CS_ANTI_HACK.h"
#include "GameClient/TS_CS_ARRANGE_ITEM.h"
#include "GameClient/TS_CS_ATTACK_REQUEST.h"
#include "GameClient/TS_CS_AUCTION_BID.h"
#include "GameClient/TS_CS_AUCTION_BIDDED_LIST.h"
#include "GameClient/TS_CS_AUCTION_CANCEL.h"
#include "GameClient/TS_CS_AUCTION_INSTANT_PURCHASE.h"
#include "GameClient/TS_CS_AUCTION_REGISTER.h"
#include "GameClient/TS_CS_AUCTION_SEARCH.h"
#include "GameClient/TS_CS_AUCTION_SELLING_LIST.h"
#include "GameClient/TS_CS_BATTLE_ARENA_ABSENCE_CHECK_ANSWER.h"
#include "GameClient/TS_CS_BATTLE_ARENA_ABSENCE_CHECK_REQUEST.h"
#include "GameClient/TS_CS_BATTLE_ARENA_ENTER_WHILE_COUNTDOWN.h"
#include "GameClient/TS_CS_BATTLE_ARENA_EXERCISE_READY.h"
#include "GameClient/TS_CS_BATTLE_ARENA_EXERCISE_START.h"
#include "GameClient/TS_CS_BATTLE_ARENA_JOIN_QUEUE.h"
#include "GameClient/TS_CS_BATTLE_ARENA_LEAVE.h"
#include "GameClient/TS_CS_BIND_SKILLCARD.h"
#include "GameClient/TS_CS_BOOKMARK_TITLE.h"
#include "GameClient/TS_CS_BUY_FROM_BOOTH.h"
#include "GameClient/TS_CS_BUY_ITEM.h"
#include "GameClient/TS_CS_CANCEL_ACTION.h"
#include "GameClient/TS_CS_CHANGE_ALIAS.h"
#include "GameClient/TS_CS_CHANGE_ITEM_POSITION.h"
#include "GameClient/TS_CS_CHANGE_LOCATION.h"
#include "GameClient/TS_CS_CHANGE_SUMMON_NAME.h"
#include "GameClient/TS_CS_CHARACTER_LIST.h"
#include "GameClient/TS_CS_CHAT_REQUEST.h"
#include "GameClient/TS_CS_CHECK_BOOTH_STARTABLE.h"
#include "GameClient/TS_CS_CHECK_CHARACTER_NAME.h"
#include "GameClient/TS_CS_CHECK_ILLEGAL_USER.h"
#include "GameClient/TS_CS_COMPETE_ANSWER.h"
#include "GameClient/TS_CS_COMPETE_REQUEST.h"
#include "GameClient/TS_CS_CONTACT.h"
#include "GameClient/TS_CS_CREATE_CHARACTER.h"
#include "GameClient/TS_CS_DECOMPOSE.h"
#include "GameClient/TS_CS_DELETE_CHARACTER.h"
#include "GameClient/TS_CS_DIALOG.h"
#include "GameClient/TS_CS_DONATE_ITEM.h"
#include "GameClient/TS_CS_DONATE_REWARD.h"
#include "GameClient/TS_CS_DROP_ITEM.h"
#include "GameClient/TS_CS_DROP_QUEST.h"
#include "GameClient/TS_CS_EMOTION.h"
#include "GameClient/TS_CS_END_QUEST.h"
#include "GameClient/TS_CS_ENTER_EVENT_AREA.h"
#include "GameClient/TS_CS_ERASE_ITEM.h"
#include "GameClient/TS_CS_FOSTER_CREATURE.h"
#include "GameClient/TS_CS_GAME_GUARD_AUTH_ANSWER.h"
#include "GameClient/TS_CS_GAME_TIME.h"
#include "GameClient/TS_CS_GET_BOOTHS_NAME.h"
#include "GameClient/TS_CS_GET_REGION_INFO.h"
#include "GameClient/TS_CS_GET_SUMMON_SETUP_INFO.h"
#include "GameClient/TS_CS_GET_WEATHER_INFO.h"
#include "GameClient/TS_CS_GROUP_FINDER_LIST.h"
#include "GameClient/TS_CS_HIDE_EQUIP_INFO.h"
#include "GameClient/TS_CS_HUNTAHOLIC_BEGIN_HUNTING.h"
#include "GameClient/TS_CS_HUNTAHOLIC_CREATE_INSTANCE.h"
#include "GameClient/TS_CS_HUNTAHOLIC_INSTANCE_LIST.h"
#include "GameClient/TS_CS_HUNTAHOLIC_JOIN_INSTANCE.h"
#include "GameClient/TS_CS_HUNTAHOLIC_LEAVE_INSTANCE.h"
#include "GameClient/TS_CS_HUNTAHOLIC_LEAVE_LOBBY.h"
#include "GameClient/TS_CS_INSTANCE_GAME_ENTER.h"
#include "GameClient/TS_CS_INSTANCE_GAME_EXIT.h"
#include "GameClient/TS_CS_INSTANCE_GAME_SCORE_REQUEST.h"
#include "GameClient/TS_CS_ITEM_KEEPING_LIST.h"
#include "GameClient/TS_CS_ITEM_KEEPING_TAKE.h"
#include "GameClient/TS_CS_JOB_LEVEL_UP.h"
#include "GameClient/TS_CS_LEARN_SKILL.h"
#include "GameClient/TS_CS_LEAVE_EVENT_AREA.h"
#include "GameClient/TS_CS_LOGIN.h"
#include "GameClient/TS_CS_LOGOUT.h"
#include "GameClient/TS_CS_MIX.h"
#include "GameClient/TS_CS_MONSTER_RECOGNIZE.h"
#include "GameClient/TS_CS_MOVE_REQUEST.h"
#include "GameClient/TS_CS_NURSE_CREATURE.h"
#include "GameClient/TS_CS_OPEN_ITEM_SHOP.h"
#include "GameClient/TS_CS_PUTOFF_CARD.h"
#include "GameClient/TS_CS_PUTOFF_ITEM.h"
#include "GameClient/TS_CS_PUTON_CARD.h"
#include "GameClient/TS_CS_PUTON_ITEM.h"
#include "GameClient/TS_CS_PUTON_ITEM_SET.h"
#include "GameClient/TS_CS_QUERY.h"
#include "GameClient/TS_CS_QUEST_INFO.h"
#include "GameClient/TS_CS_RANKING_TOP_RECORD.h"
#include "GameClient/TS_CS_REGION_UPDATE.h"
#include "GameClient/TS_CS_REPAIR_SOULSTONE.h"
#include "GameClient/TS_CS_REPORT.h"
#include "GameClient/TS_CS_REQUEST.h"
#include "GameClient/TS_CS_REQUEST_FARM_INFO.h"
#include "GameClient/TS_CS_REQUEST_FARM_MARKET.h"
#include "GameClient/TS_CS_REQUEST_LOGOUT.h"
#include "GameClient/TS_CS_REQUEST_REMOVE_STATE.h"
#include "GameClient/TS_CS_REQUEST_RETURN_LOBBY.h"
#include "GameClient/TS_CS_RESURRECTION.h"
#include "GameClient/TS_CS_RETRIEVE_CREATURE.h"
#include "GameClient/TS_CS_RETURN_LOBBY.h"
#include "GameClient/TS_CS_SECURITY_NO.h"
#include "GameClient/TS_CS_SELL_ITEM.h"
#include "GameClient/TS_CS_SELL_TO_BOOTH.h"
#include "GameClient/TS_CS_SET_MAIN_TITLE.h"
#include "GameClient/TS_CS_SET_PET_NAME.h"
#include "GameClient/TS_CS_SET_PROPERTY.h"
#include "GameClient/TS_CS_SET_SUB_TITLE.h"
#include "GameClient/TS_CS_SKILL.h"
#include "GameClient/TS_CS_SOULSTONE_CRAFT.h"
#include "GameClient/TS_CS_START_BOOTH.h"
#include "GameClient/TS_CS_STOP_BOOTH.h"
#include "GameClient/TS_CS_STOP_WATCH_BOOTH.h"
#include "GameClient/TS_CS_STORAGE.h"
#include "GameClient/TS_CS_SUMMON.h"
#include "GameClient/TS_CS_SUMMON_CARD_SKILL_LIST.h"
#include "GameClient/TS_CS_SWAP_EQUIP.h"
#include "GameClient/TS_CS_TAKEOUT_COMMERCIAL_ITEM.h"
#include "GameClient/TS_CS_TAKE_ITEM.h"
#include "GameClient/TS_CS_TARGETING.h"
#include "GameClient/TS_CS_TRANSMIT_ETHEREAL_DURABILITY.h"
#include "GameClient/TS_CS_TRANSMIT_ETHEREAL_DURABILITY_TO_EQUIPMENT.h"
#include "GameClient/TS_CS_TURN_OFF_PK_MODE.h"
#include "GameClient/TS_CS_TURN_ON_PK_MODE.h"
#include "GameClient/TS_CS_UNBIND_SKILLCARD.h"
#include "GameClient/TS_CS_UPDATE.h"
#include "GameClient/TS_CS_USE_ITEM.h"
#include "GameClient/TS_CS_VERSION.h"
#include "GameClient/TS_CS_WATCH_BOOTH.h"
#include "GameClient/TS_CS_XTRAP_CHECK.h"
#include "GameClient/TS_EQUIP_SUMMON.h"
#include "GameClient/TS_SC_ACHIEVE_TITLE.h"
#include "GameClient/TS_SC_ADDED_SKILL_LIST.h"
#include "GameClient/TS_SC_ADD_PET_INFO.h"
#include "GameClient/TS_SC_ADD_SUMMON_INFO.h"
#include "GameClient/TS_SC_ANTI_HACK.h"
#include "GameClient/TS_SC_ATTACK_EVENT.h"
#include "GameClient/TS_SC_AUCTION_BIDDED_LIST.h"
#include "GameClient/TS_SC_AUCTION_SEARCH.h"
#include "GameClient/TS_SC_AUCTION_SELLING_LIST.h"
#include "GameClient/TS_SC_AURA.h"
#include "GameClient/TS_SC_BATTLE_ARENA_ABSENCE_CHECK.h"
#include "GameClient/TS_SC_BATTLE_ARENA_BATTLE_INFO.h"
#include "GameClient/TS_SC_BATTLE_ARENA_BATTLE_SCORE.h"
#include "GameClient/TS_SC_BATTLE_ARENA_BATTLE_STATUS.h"
#include "GameClient/TS_SC_BATTLE_ARENA_DISCONNECT_BATTLE.h"
#include "GameClient/TS_SC_BATTLE_ARENA_EXERCISE_READY_STATUS.h"
#include "GameClient/TS_SC_BATTLE_ARENA_JOIN_BATTLE.h"
#include "GameClient/TS_SC_BATTLE_ARENA_JOIN_QUEUE.h"
#include "GameClient/TS_SC_BATTLE_ARENA_LEAVE.h"
#include "GameClient/TS_SC_BATTLE_ARENA_PENALTY_INFO.h"
#include "GameClient/TS_SC_BATTLE_ARENA_RECONNECT_BATTLE.h"
#include "GameClient/TS_SC_BATTLE_ARENA_RESULT.h"
#include "GameClient/TS_SC_BATTLE_ARENA_UPDATE_WAIT_USER_COUNT.h"
#include "GameClient/TS_SC_BELT_SLOT_INFO.h"
#include "GameClient/TS_SC_BONUS_EXP_JP.h"
#include "GameClient/TS_SC_BOOKMARK_TITLE.h"
#include "GameClient/TS_SC_BOOTH_CLOSED.h"
#include "GameClient/TS_SC_BOOTH_TRADE_INFO.h"
#include "GameClient/TS_SC_CANT_ATTACK.h"
#include "GameClient/TS_SC_CHANGE_LOCATION.h"
#include "GameClient/TS_SC_CHANGE_NAME.h"
#include "GameClient/TS_SC_CHANGE_TITLE_CONDITION.h"
#include "GameClient/TS_SC_CHARACTER_LIST.h"
#include "GameClient/TS_SC_CHAT.h"
#include "GameClient/TS_SC_CHAT_LOCAL.h"
#include "GameClient/TS_SC_CHAT_RESULT.h"
#include "GameClient/TS_SC_COMMERCIAL_STORAGE_INFO.h"
#include "GameClient/TS_SC_COMMERCIAL_STORAGE_LIST.h"
#include "GameClient/TS_SC_COMPETE_ANSWER.h"
#include "GameClient/TS_SC_COMPETE_COUNTDOWN.h"
#include "GameClient/TS_SC_COMPETE_END.h"
#include "GameClient/TS_SC_COMPETE_REQUEST.h"
#include "GameClient/TS_SC_COMPETE_START.h"
#include "GameClient/TS_SC_DECOMPOSE_RESULT.h"
#include "GameClient/TS_SC_DESTROY_ITEM.h"
#include "GameClient/TS_SC_DETECT_RANGE_UPDATE.h"
#include "GameClient/TS_SC_DIALOG.h"
#include "GameClient/TS_SC_DISCONNECT_DESC.h"
#include "GameClient/TS_SC_DROP_RESULT.h"
#include "GameClient/TS_SC_EMOTION.h"
#include "GameClient/TS_SC_ENERGY.h"
#include "GameClient/TS_SC_ENTER.h"
#include "GameClient/TS_SC_ERASE_ITEM.h"
#include "GameClient/TS_SC_EXP_UPDATE.h"
#include "GameClient/TS_SC_FARM_INFO.h"
#include "GameClient/TS_SC_GAME_GUARD_AUTH_QUERY.h"
#include "GameClient/TS_SC_GAME_TIME.h"
#include "GameClient/TS_SC_GENERAL_MESSAGE_BOX.h"
#include "GameClient/TS_SC_GET_BOOTHS_NAME.h"
#include "GameClient/TS_SC_GET_CHAOS.h"
#include "GameClient/TS_SC_GOLD_UPDATE.h"
#include "GameClient/TS_SC_GROUP_FINDER_DETAIL.h"
#include "GameClient/TS_SC_GROUP_FINDER_LIST.h"
#include "GameClient/TS_SC_HAIR_INFO.h"
#include "GameClient/TS_SC_HIDE_EQUIP_INFO.h"
#include "GameClient/TS_SC_HPMP.h"
#include "GameClient/TS_SC_HUNTAHOLIC_BEGIN_COUNTDOWN.h"
#include "GameClient/TS_SC_HUNTAHOLIC_BEGIN_HUNTING.h"
#include "GameClient/TS_SC_HUNTAHOLIC_HUNTING_SCORE.h"
#include "GameClient/TS_SC_HUNTAHOLIC_INSTANCE_INFO.h"
#include "GameClient/TS_SC_HUNTAHOLIC_INSTANCE_LIST.h"
#include "GameClient/TS_SC_HUNTAHOLIC_MAX_POINT_ACHIEVED.h"
#include "GameClient/TS_SC_HUNTAHOLIC_UPDATE_SCORE.h"
#include "GameClient/TS_SC_INSTANCE_GAME_SCORE_REQUEST.h"
#include "GameClient/TS_SC_INVENTORY.h"
#include "GameClient/TS_SC_ITEM_COOL_TIME.h"
#include "GameClient/TS_SC_ITEM_DROP_INFO.h"
#include "GameClient/TS_SC_ITEM_KEEPING_LIST.h"
#include "GameClient/TS_SC_ITEM_WEAR_INFO.h"
#include "GameClient/TS_SC_LEAVE.h"
#include "GameClient/TS_SC_LEVEL_UPDATE.h"
#include "GameClient/TS_SC_LOGIN_RESULT.h"
#include "GameClient/TS_SC_MARKET.h"
#include "GameClient/TS_SC_MIX_RESULT.h"
#include "GameClient/TS_SC_MOUNT_SUMMON.h"
#include "GameClient/TS_SC_MOVE.h"
#include "GameClient/TS_SC_MOVE_ACK.h"
#include "GameClient/TS_SC_NPC_TRADE_INFO.h"
#include "GameClient/TS_SC_OPEN_GUILD_WINDOW.h"
#include "GameClient/TS_SC_OPEN_ITEM_SHOP.h"
#include "GameClient/TS_SC_OPEN_STORAGE.h"
#include "GameClient/TS_SC_OPEN_TITLE.h"
#include "GameClient/TS_SC_OPEN_URL.h"
#include "GameClient/TS_SC_PROPERTY.h"
#include "GameClient/TS_SC_QUEST_INFOMATION.h"
#include "GameClient/TS_SC_QUEST_LIST.h"
#include "GameClient/TS_SC_QUEST_STATUS.h"
#include "GameClient/TS_SC_RANKING_TOP_RECORD.h"
#include "GameClient/TS_SC_REGEN_HPMP.h"
#include "GameClient/TS_SC_REGEN_INFO.h"
#include "GameClient/TS_SC_REGION_ACK.h"
#include "GameClient/TS_SC_REMAIN_TITLE_TIME.h"
#include "GameClient/TS_SC_REMOVE_PET_INFO.h"
#include "GameClient/TS_SC_REMOVE_SUMMON_INFO.h"
#include "GameClient/TS_SC_REQUEST_SECURITY_NO.h"
#include "GameClient/TS_SC_RESULT.h"
#include "GameClient/TS_SC_RESULT_FOSTER.h"
#include "GameClient/TS_SC_RESULT_NURSE.h"
#include "GameClient/TS_SC_RESULT_RETRIEVE.h"
#include "GameClient/TS_SC_SET_MAIN_TITLE.h"
#include "GameClient/TS_SC_SET_SUB_TITLE.h"
#include "GameClient/TS_SC_SET_TIME.h"
#include "GameClient/TS_SC_SHOW_CREATE_ALLIANCE.h"
#include "GameClient/TS_SC_SHOW_CREATE_GUILD.h"
#include "GameClient/TS_SC_SHOW_SET_PET_NAME.h"
#include "GameClient/TS_SC_SHOW_SOULSTONE_CRAFT_WINDOW.h"
#include "GameClient/TS_SC_SHOW_SOULSTONE_REPAIR_WINDOW.h"
#include "GameClient/TS_SC_SHOW_SUMMON_NAME_CHANGE.h"
#include "GameClient/TS_SC_SHOW_WINDOW.h"
#include "GameClient/TS_SC_SKILL.h"
#include "GameClient/TS_SC_SKILLCARD_INFO.h"
#include "GameClient/TS_SC_SKILL_LEVEL_LIST.h"
#include "GameClient/TS_SC_SKILL_LIST.h"
#include "GameClient/TS_SC_SP.h"
#include "GameClient/TS_SC_STATE.h"
#include "GameClient/TS_SC_STATE_RESULT.h"
#include "GameClient/TS_SC_STATUS_CHANGE.h"
#include "GameClient/TS_SC_STAT_INFO.h"
#include "GameClient/TS_SC_SUMMON_EVOLUTION.h"
#include "GameClient/TS_SC_TAKE_ITEM_RESULT.h"
#include "GameClient/TS_SC_TAMING_INFO.h"
#include "GameClient/TS_SC_TARGET.h"
#include "GameClient/TS_SC_TITLE_CONDITION_LIST.h"
#include "GameClient/TS_SC_TITLE_LIST.h"
#include "GameClient/TS_SC_UNMOUNT_SUMMON.h"
#include "GameClient/TS_SC_UNSUMMON.h"
#include "GameClient/TS_SC_UNSUMMON_NOTICE.h"
#include "GameClient/TS_SC_UNSUMMON_PET.h"
#include "GameClient/TS_SC_UPDATE_GUILD_ICON.h"
#include "GameClient/TS_SC_UPDATE_ITEM_COUNT.h"
#include "GameClient/TS_SC_URL_LIST.h"
#include "GameClient/TS_SC_USE_ITEM_RESULT.h"
#include "GameClient/TS_SC_WARP.h"
#include "GameClient/TS_SC_WATCH_BOOTH.h"
#include "GameClient/TS_SC_WEAR_INFO.h"
#include "GameClient/TS_SC_WEATHER_INFO.h"
#include "GameClient/TS_SC_XTRAP_CHECK.h"
#include "GameClient/TS_TIMESYNC.h"
#include "GameClient/TS_TRADE.h"

template<template<typename> class Functor, typename... Args> bool processAuthPacket(int packetId, Args... args) {
#define SEND_PACKET(_type) \
	case _type::packetID: \
		Functor<_type>()(args...); \
		break

	switch(packetId) {
		SEND_PACKET(TS_AC_ACCOUNT_NAME);
		SEND_PACKET(TS_AC_AES_KEY_IV);
		SEND_PACKET(TS_AC_RESULT);
		SEND_PACKET(TS_AC_RESULT_WITH_STRING);
		SEND_PACKET(TS_AC_SELECT_SERVER);
		SEND_PACKET(TS_AC_SERVER_LIST);
		SEND_PACKET(TS_AC_UPDATE_PENDING_TIME);
		SEND_PACKET(TS_CA_ACCOUNT);
		SEND_PACKET(TS_CA_DISTRIBUTION_INFO);
		SEND_PACKET(TS_CA_IMBC_ACCOUNT);
		SEND_PACKET(TS_CA_OTP_ACCOUNT);
		SEND_PACKET(TS_CA_RSA_PUBLIC_KEY);
		SEND_PACKET(TS_CA_SELECT_SERVER);
		SEND_PACKET(TS_CA_SERVER_LIST);
		SEND_PACKET(TS_CA_VERSION);

		SEND_PACKET(TS_SC_RESULT);

		default:
			return false;
	}

#undef SEND_PACKET

	return true;
}

template<template<typename> class Functor, typename... Args>
bool processGamePacket(int packetId, bool isServerPacket, Args... args) {
#define SEND_PACKET(_type) \
	case _type::packetID: \
		Functor<_type>()(args...); \
		break

#define SEND_PACKET_2(_type) \
	case_packet_is(_type) Functor<_type>()(args...); \
	break

#define SEND_PACKET_CLISERV(_isServer, _type_serv, _type_client) \
	case _type_serv::packetID: \
		static_assert(_type_serv::packetID == _type_client::packetID, "expected same packet ID"); \
		(void) (sizeof(&_type_serv::getSize)); \
		(void) (sizeof(&_type_client::getSize)); \
		if(_isServer) \
			Functor<_type_serv>()(args...); \
		else \
			Functor<_type_client>()(args...); \
		break

	switch(packetId) {
		SEND_PACKET(TS_CS_ACCOUNT_WITH_AUTH);
		SEND_PACKET(TS_CS_ANTI_HACK);
		SEND_PACKET(TS_CS_ARRANGE_ITEM);
		SEND_PACKET(TS_CS_ATTACK_REQUEST);
		SEND_PACKET(TS_CS_AUCTION_BIDDED_LIST);
		SEND_PACKET(TS_CS_AUCTION_BID);
		SEND_PACKET(TS_CS_AUCTION_CANCEL);
		SEND_PACKET(TS_CS_AUCTION_INSTANT_PURCHASE);
		SEND_PACKET(TS_CS_AUCTION_REGISTER);
		SEND_PACKET(TS_CS_AUCTION_SEARCH);
		SEND_PACKET(TS_CS_AUCTION_SELLING_LIST);
		SEND_PACKET(TS_CS_BATTLE_ARENA_ABSENCE_CHECK_ANSWER);
		SEND_PACKET(TS_CS_BATTLE_ARENA_ABSENCE_CHECK_REQUEST);
		SEND_PACKET(TS_CS_BATTLE_ARENA_ENTER_WHILE_COUNTDOWN);
		SEND_PACKET(TS_CS_BATTLE_ARENA_EXERCISE_READY);
		SEND_PACKET(TS_CS_BATTLE_ARENA_EXERCISE_START);
		SEND_PACKET(TS_CS_BATTLE_ARENA_JOIN_QUEUE);
		SEND_PACKET(TS_CS_BATTLE_ARENA_LEAVE);
		SEND_PACKET(TS_CS_BIND_SKILLCARD);
		SEND_PACKET(TS_CS_BOOKMARK_TITLE);
		SEND_PACKET(TS_CS_BUY_FROM_BOOTH);
		SEND_PACKET(TS_CS_BUY_ITEM);
		SEND_PACKET(TS_CS_CANCEL_ACTION);
		SEND_PACKET(TS_CS_CHANGE_ALIAS);
		SEND_PACKET(TS_CS_CHANGE_ITEM_POSITION);
		SEND_PACKET(TS_CS_CHANGE_LOCATION);
		SEND_PACKET(TS_CS_CHANGE_SUMMON_NAME);
		SEND_PACKET_2(TS_CS_CHARACTER_LIST);
		SEND_PACKET(TS_CS_CHAT_REQUEST);
		SEND_PACKET(TS_CS_CHECK_BOOTH_STARTABLE);
		SEND_PACKET(TS_CS_CHECK_CHARACTER_NAME);
		SEND_PACKET(TS_CS_CHECK_ILLEGAL_USER);
		SEND_PACKET(TS_CS_COMPETE_ANSWER);
		SEND_PACKET(TS_CS_COMPETE_REQUEST);
		SEND_PACKET(TS_CS_CONTACT);
		SEND_PACKET(TS_CS_CREATE_CHARACTER);
		SEND_PACKET(TS_CS_DECOMPOSE);
		SEND_PACKET(TS_CS_DELETE_CHARACTER);
		SEND_PACKET(TS_CS_DIALOG);
		SEND_PACKET(TS_CS_DONATE_ITEM);
		SEND_PACKET(TS_CS_DROP_ITEM);
		SEND_PACKET(TS_CS_DROP_QUEST);
		SEND_PACKET(TS_CS_EMOTION);
		SEND_PACKET(TS_CS_END_QUEST);
		SEND_PACKET(TS_CS_ENTER_EVENT_AREA);
		SEND_PACKET(TS_CS_ERASE_ITEM);
		SEND_PACKET(TS_CS_FOSTER_CREATURE);
		SEND_PACKET(TS_CS_GAME_GUARD_AUTH_ANSWER);
		SEND_PACKET(TS_CS_GAME_TIME);
		SEND_PACKET(TS_CS_GET_BOOTHS_NAME);
		SEND_PACKET(TS_CS_GET_REGION_INFO);
		SEND_PACKET(TS_CS_GET_SUMMON_SETUP_INFO);
		SEND_PACKET(TS_CS_GET_WEATHER_INFO);
		SEND_PACKET(TS_CS_GROUP_FINDER_LIST);
		SEND_PACKET(TS_CS_HIDE_EQUIP_INFO);
		SEND_PACKET(TS_CS_HUNTAHOLIC_BEGIN_HUNTING);
		SEND_PACKET(TS_CS_HUNTAHOLIC_CREATE_INSTANCE);
		SEND_PACKET(TS_CS_HUNTAHOLIC_INSTANCE_LIST);
		SEND_PACKET(TS_CS_HUNTAHOLIC_JOIN_INSTANCE);
		SEND_PACKET(TS_CS_HUNTAHOLIC_LEAVE_INSTANCE);
		SEND_PACKET(TS_CS_HUNTAHOLIC_LEAVE_LOBBY);
		SEND_PACKET(TS_CS_INSTANCE_GAME_ENTER);
		SEND_PACKET(TS_CS_INSTANCE_GAME_EXIT);
		SEND_PACKET(TS_CS_INSTANCE_GAME_SCORE_REQUEST);
		SEND_PACKET(TS_CS_ITEM_KEEPING_LIST);
		SEND_PACKET(TS_CS_ITEM_KEEPING_TAKE);
		SEND_PACKET(TS_CS_JOB_LEVEL_UP);
		SEND_PACKET(TS_CS_LEARN_SKILL);
		SEND_PACKET(TS_CS_LEAVE_EVENT_AREA);
		SEND_PACKET_2(TS_CS_LOGIN);
		SEND_PACKET(TS_CS_LOGOUT);
		SEND_PACKET(TS_CS_MIX);
		SEND_PACKET(TS_CS_MONSTER_RECOGNIZE);
		SEND_PACKET_2(TS_CS_MOVE_REQUEST);
		SEND_PACKET(TS_CS_NURSE_CREATURE);
		SEND_PACKET(TS_CS_OPEN_ITEM_SHOP);
		SEND_PACKET(TS_CS_PUTOFF_CARD);
		SEND_PACKET(TS_CS_PUTOFF_ITEM);
		SEND_PACKET(TS_CS_PUTON_CARD);
		SEND_PACKET(TS_CS_PUTON_ITEM);
		SEND_PACKET(TS_CS_PUTON_ITEM_SET);
		SEND_PACKET(TS_CS_QUERY);
		SEND_PACKET(TS_CS_QUEST_INFO);
		SEND_PACKET(TS_CS_RANKING_TOP_RECORD);
		SEND_PACKET_2(TS_CS_REGION_UPDATE);
		SEND_PACKET(TS_CS_REPAIR_SOULSTONE);
		SEND_PACKET(TS_CS_REPORT);
		SEND_PACKET(TS_CS_REQUEST);
		SEND_PACKET(TS_CS_REQUEST_FARM_INFO);
		SEND_PACKET(TS_CS_REQUEST_FARM_MARKET);
		SEND_PACKET(TS_CS_REQUEST_LOGOUT);
		SEND_PACKET(TS_CS_REQUEST_REMOVE_STATE);
		SEND_PACKET(TS_CS_REQUEST_RETURN_LOBBY);
		SEND_PACKET(TS_CS_RESURRECTION);
		SEND_PACKET(TS_CS_RETRIEVE_CREATURE);
		SEND_PACKET(TS_CS_RETURN_LOBBY);
		SEND_PACKET(TS_CS_SECURITY_NO);
		SEND_PACKET(TS_CS_SELL_ITEM);
		SEND_PACKET(TS_CS_SELL_TO_BOOTH);
		SEND_PACKET(TS_CS_SET_MAIN_TITLE);
		SEND_PACKET(TS_CS_SET_PET_NAME);
		SEND_PACKET(TS_CS_SET_PROPERTY);
		SEND_PACKET(TS_CS_SET_SUB_TITLE);
		SEND_PACKET(TS_CS_SKILL);
		SEND_PACKET(TS_CS_SOULSTONE_CRAFT);
		SEND_PACKET(TS_CS_START_BOOTH);
		SEND_PACKET(TS_CS_STOP_BOOTH);
		SEND_PACKET(TS_CS_STOP_WATCH_BOOTH);
		SEND_PACKET(TS_CS_STORAGE);
		SEND_PACKET(TS_CS_SUMMON_CARD_SKILL_LIST);
		SEND_PACKET(TS_CS_SUMMON);
		SEND_PACKET(TS_CS_SWAP_EQUIP);
		SEND_PACKET(TS_CS_TAKE_ITEM);
		SEND_PACKET(TS_CS_TAKEOUT_COMMERCIAL_ITEM);
		SEND_PACKET(TS_CS_TARGETING);
		SEND_PACKET(TS_CS_TRANSMIT_ETHEREAL_DURABILITY);
		SEND_PACKET(TS_CS_TRANSMIT_ETHEREAL_DURABILITY_TO_EQUIPMENT);
		SEND_PACKET(TS_CS_TURN_OFF_PK_MODE);
		SEND_PACKET(TS_CS_TURN_ON_PK_MODE);
		SEND_PACKET(TS_CS_UNBIND_SKILLCARD);
		SEND_PACKET(TS_CS_UPDATE);
		SEND_PACKET(TS_CS_USE_ITEM);
		SEND_PACKET_2(TS_CS_VERSION);
		SEND_PACKET(TS_CS_WATCH_BOOTH);
		SEND_PACKET(TS_CS_XTRAP_CHECK);
		SEND_PACKET(TS_EQUIP_SUMMON);
		SEND_PACKET(TS_SC_ACHIEVE_TITLE);
		SEND_PACKET(TS_SC_ADDED_SKILL_LIST);
		SEND_PACKET(TS_SC_ADD_PET_INFO);
		SEND_PACKET(TS_SC_ADD_SUMMON_INFO);
		SEND_PACKET(TS_SC_ANTI_HACK);
		SEND_PACKET(TS_SC_ATTACK_EVENT);
		SEND_PACKET(TS_SC_AUCTION_BIDDED_LIST);
		SEND_PACKET(TS_SC_AUCTION_SEARCH);
		SEND_PACKET(TS_SC_AUCTION_SELLING_LIST);
		SEND_PACKET(TS_SC_AURA);
		SEND_PACKET(TS_SC_BATTLE_ARENA_ABSENCE_CHECK);
		SEND_PACKET(TS_SC_BATTLE_ARENA_BATTLE_INFO);
		SEND_PACKET(TS_SC_BATTLE_ARENA_BATTLE_SCORE);
		SEND_PACKET(TS_SC_BATTLE_ARENA_BATTLE_STATUS);
		SEND_PACKET(TS_SC_BATTLE_ARENA_DISCONNECT_BATTLE);
		SEND_PACKET(TS_SC_BATTLE_ARENA_EXERCISE_READY_STATUS);
		SEND_PACKET(TS_SC_BATTLE_ARENA_JOIN_BATTLE);
		SEND_PACKET(TS_SC_BATTLE_ARENA_JOIN_QUEUE);
		SEND_PACKET(TS_SC_BATTLE_ARENA_LEAVE);
		SEND_PACKET(TS_SC_BATTLE_ARENA_PENALTY_INFO);
		SEND_PACKET(TS_SC_BATTLE_ARENA_RECONNECT_BATTLE);
		SEND_PACKET(TS_SC_BATTLE_ARENA_RESULT);
		SEND_PACKET(TS_SC_BATTLE_ARENA_UPDATE_WAIT_USER_COUNT);
		SEND_PACKET(TS_SC_BELT_SLOT_INFO);
		SEND_PACKET(TS_SC_BONUS_EXP_JP);
		SEND_PACKET(TS_SC_BOOKMARK_TITLE);
		SEND_PACKET(TS_SC_BOOTH_CLOSED);
		SEND_PACKET(TS_SC_BOOTH_TRADE_INFO);
		SEND_PACKET(TS_SC_CANT_ATTACK);
		SEND_PACKET(TS_SC_CHANGE_LOCATION);
		SEND_PACKET(TS_SC_CHANGE_NAME);
		SEND_PACKET(TS_SC_CHANGE_TITLE_CONDITION);
		SEND_PACKET(TS_SC_CHARACTER_LIST);
		SEND_PACKET(TS_SC_CHAT);
		SEND_PACKET(TS_SC_CHAT_LOCAL);
		SEND_PACKET(TS_SC_CHAT_RESULT);
		SEND_PACKET(TS_SC_COMMERCIAL_STORAGE_INFO);
		SEND_PACKET(TS_SC_COMMERCIAL_STORAGE_LIST);
		SEND_PACKET(TS_SC_COMPETE_ANSWER);
		SEND_PACKET(TS_SC_COMPETE_COUNTDOWN);
		SEND_PACKET(TS_SC_COMPETE_END);
		SEND_PACKET(TS_SC_COMPETE_REQUEST);
		SEND_PACKET(TS_SC_COMPETE_START);
		SEND_PACKET(TS_SC_DECOMPOSE_RESULT);
		SEND_PACKET(TS_SC_DESTROY_ITEM);
		SEND_PACKET(TS_SC_DETECT_RANGE_UPDATE);
		SEND_PACKET(TS_SC_DIALOG);
		SEND_PACKET(TS_SC_DISCONNECT_DESC);
		SEND_PACKET(TS_SC_DROP_RESULT);
		SEND_PACKET(TS_SC_EMOTION);
		SEND_PACKET(TS_SC_ENERGY);
		SEND_PACKET_2(TS_SC_ENTER);
		SEND_PACKET(TS_SC_ERASE_ITEM);
		SEND_PACKET(TS_SC_EXP_UPDATE);
		SEND_PACKET(TS_SC_FARM_INFO);
		SEND_PACKET(TS_SC_GAME_GUARD_AUTH_QUERY);
		SEND_PACKET(TS_SC_GAME_TIME);
		SEND_PACKET(TS_SC_GENERAL_MESSAGE_BOX);
		SEND_PACKET(TS_SC_GET_BOOTHS_NAME);
		SEND_PACKET(TS_SC_GET_CHAOS);
		SEND_PACKET(TS_SC_GOLD_UPDATE);
		SEND_PACKET(TS_SC_GROUP_FINDER_DETAIL);
		SEND_PACKET(TS_SC_GROUP_FINDER_LIST);
		SEND_PACKET(TS_SC_HAIR_INFO);
		SEND_PACKET(TS_SC_HIDE_EQUIP_INFO);
		SEND_PACKET(TS_SC_HPMP);
		SEND_PACKET(TS_SC_HUNTAHOLIC_BEGIN_COUNTDOWN);
		SEND_PACKET(TS_SC_HUNTAHOLIC_BEGIN_HUNTING);
		SEND_PACKET(TS_SC_HUNTAHOLIC_HUNTING_SCORE);
		SEND_PACKET(TS_SC_HUNTAHOLIC_INSTANCE_INFO);
		SEND_PACKET(TS_SC_HUNTAHOLIC_INSTANCE_LIST);
		SEND_PACKET(TS_SC_HUNTAHOLIC_MAX_POINT_ACHIEVED);
		SEND_PACKET(TS_SC_HUNTAHOLIC_UPDATE_SCORE);
		SEND_PACKET(TS_SC_INSTANCE_GAME_SCORE_REQUEST);
		SEND_PACKET(TS_SC_INVENTORY);
		SEND_PACKET(TS_SC_ITEM_COOL_TIME);
		SEND_PACKET(TS_SC_ITEM_DROP_INFO);
		SEND_PACKET(TS_SC_ITEM_KEEPING_LIST);
		SEND_PACKET(TS_SC_ITEM_WEAR_INFO);
		SEND_PACKET(TS_SC_LEAVE);
		SEND_PACKET(TS_SC_LEVEL_UPDATE);
		SEND_PACKET_2(TS_SC_LOGIN_RESULT);
		SEND_PACKET(TS_SC_MARKET);
		SEND_PACKET(TS_SC_MIX_RESULT);
		SEND_PACKET(TS_SC_MOUNT_SUMMON);
		SEND_PACKET_2(TS_SC_MOVE_ACK);
		SEND_PACKET(TS_SC_MOVE);
		SEND_PACKET(TS_SC_NPC_TRADE_INFO);
		SEND_PACKET(TS_SC_OPEN_GUILD_WINDOW);
		SEND_PACKET(TS_SC_OPEN_ITEM_SHOP);
		SEND_PACKET(TS_SC_OPEN_STORAGE);
		SEND_PACKET(TS_SC_OPEN_TITLE);
		SEND_PACKET(TS_SC_OPEN_URL);
		SEND_PACKET(TS_SC_PROPERTY);
		SEND_PACKET(TS_SC_QUEST_INFOMATION);
		SEND_PACKET(TS_SC_QUEST_LIST);
		SEND_PACKET(TS_SC_QUEST_STATUS);
		SEND_PACKET(TS_SC_RANKING_TOP_RECORD);
		SEND_PACKET(TS_SC_REGEN_HPMP);
		SEND_PACKET(TS_SC_REGEN_INFO);
		SEND_PACKET(TS_SC_REGION_ACK);
		SEND_PACKET(TS_SC_REMAIN_TITLE_TIME);
		SEND_PACKET(TS_SC_REMOVE_PET_INFO);
		SEND_PACKET(TS_SC_REMOVE_SUMMON_INFO);
		SEND_PACKET(TS_SC_REQUEST_SECURITY_NO);
		SEND_PACKET(TS_SC_RESULT_FOSTER);
		SEND_PACKET(TS_SC_RESULT);
		SEND_PACKET(TS_SC_RESULT_NURSE);
		SEND_PACKET(TS_SC_RESULT_RETRIEVE);
		SEND_PACKET(TS_SC_SET_MAIN_TITLE);
		SEND_PACKET(TS_SC_SET_SUB_TITLE);
		SEND_PACKET(TS_SC_SET_TIME);
		SEND_PACKET(TS_SC_SHOW_CREATE_ALLIANCE);
		SEND_PACKET(TS_SC_SHOW_CREATE_GUILD);
		SEND_PACKET(TS_SC_SHOW_SET_PET_NAME);
		SEND_PACKET(TS_SC_SHOW_SOULSTONE_REPAIR_WINDOW);
		SEND_PACKET(TS_SC_SHOW_SUMMON_NAME_CHANGE);
		SEND_PACKET(TS_SC_SHOW_WINDOW);
		SEND_PACKET(TS_SC_SKILLCARD_INFO);
		SEND_PACKET(TS_SC_SKILL);
		SEND_PACKET(TS_SC_SKILL_LEVEL_LIST);
		SEND_PACKET(TS_SC_SKILL_LIST);
		SEND_PACKET(TS_SC_SP);
		SEND_PACKET(TS_SC_STATE);
		SEND_PACKET(TS_SC_STATE_RESULT);
		SEND_PACKET(TS_SC_STAT_INFO);
		SEND_PACKET(TS_SC_STATUS_CHANGE);
		SEND_PACKET(TS_SC_SUMMON_EVOLUTION);
		SEND_PACKET(TS_SC_TAKE_ITEM_RESULT);
		SEND_PACKET(TS_SC_TAMING_INFO);
		SEND_PACKET(TS_SC_TARGET);
		SEND_PACKET(TS_SC_TITLE_CONDITION_LIST);
		SEND_PACKET(TS_SC_TITLE_LIST);
		SEND_PACKET(TS_SC_UNMOUNT_SUMMON);
		SEND_PACKET(TS_SC_UNSUMMON);
		SEND_PACKET(TS_SC_UNSUMMON_NOTICE);
		SEND_PACKET(TS_SC_UNSUMMON_PET);
		SEND_PACKET(TS_SC_UPDATE_GUILD_ICON);
		SEND_PACKET(TS_SC_UPDATE_ITEM_COUNT);
		SEND_PACKET(TS_SC_URL_LIST);
		SEND_PACKET(TS_SC_USE_ITEM_RESULT);
		SEND_PACKET(TS_SC_WARP);
		SEND_PACKET(TS_SC_WATCH_BOOTH);
		SEND_PACKET(TS_SC_WEAR_INFO);
		SEND_PACKET(TS_SC_WEATHER_INFO);
		SEND_PACKET(TS_SC_XTRAP_CHECK);
		SEND_PACKET(TS_TIMESYNC);
		SEND_PACKET(TS_TRADE);

		SEND_PACKET_CLISERV(isServerPacket, TS_SC_SHOW_SOULSTONE_CRAFT_WINDOW, TS_CS_DONATE_REWARD);

		default:
			return false;
	}

#undef SEND_PACKET_2
#undef SEND_PACKET

	return true;
}

template<template<typename> class Functor, typename... Args> void iterateAllPacketTypes(Args... args) {
	// Auth packets
	Functor<TS_AC_ACCOUNT_NAME>()(args...);
	Functor<TS_AC_AES_KEY_IV>()(args...);
	Functor<TS_AC_RESULT>()(args...);
	Functor<TS_AC_RESULT_WITH_STRING>()(args...);
	Functor<TS_AC_SELECT_SERVER>()(args...);
	Functor<TS_AC_SERVER_LIST>()(args...);
	Functor<TS_AC_UPDATE_PENDING_TIME>()(args...);
	Functor<TS_CA_ACCOUNT>()(args...);
	Functor<TS_CA_DISTRIBUTION_INFO>()(args...);
	Functor<TS_CA_IMBC_ACCOUNT>()(args...);
	Functor<TS_CA_OTP_ACCOUNT>()(args...);
	Functor<TS_CA_RSA_PUBLIC_KEY>()(args...);
	Functor<TS_CA_SELECT_SERVER>()(args...);
	Functor<TS_CA_SERVER_LIST>()(args...);
	Functor<TS_CA_VERSION>()(args...);

	// GS Packets

	Functor<TS_CS_ACCOUNT_WITH_AUTH>()(args...);
	Functor<TS_CS_ANTI_HACK>()(args...);
	Functor<TS_CS_ARRANGE_ITEM>()(args...);
	Functor<TS_CS_ATTACK_REQUEST>()(args...);
	Functor<TS_CS_AUCTION_BIDDED_LIST>()(args...);
	Functor<TS_CS_AUCTION_BID>()(args...);
	Functor<TS_CS_AUCTION_CANCEL>()(args...);
	Functor<TS_CS_AUCTION_INSTANT_PURCHASE>()(args...);
	Functor<TS_CS_AUCTION_REGISTER>()(args...);
	Functor<TS_CS_AUCTION_SEARCH>()(args...);
	Functor<TS_CS_AUCTION_SELLING_LIST>()(args...);
	Functor<TS_CS_BATTLE_ARENA_ABSENCE_CHECK_ANSWER>()(args...);
	Functor<TS_CS_BATTLE_ARENA_ABSENCE_CHECK_REQUEST>()(args...);
	Functor<TS_CS_BATTLE_ARENA_ENTER_WHILE_COUNTDOWN>()(args...);
	Functor<TS_CS_BATTLE_ARENA_EXERCISE_READY>()(args...);
	Functor<TS_CS_BATTLE_ARENA_EXERCISE_START>()(args...);
	Functor<TS_CS_BATTLE_ARENA_JOIN_QUEUE>()(args...);
	Functor<TS_CS_BATTLE_ARENA_LEAVE>()(args...);
	Functor<TS_CS_BIND_SKILLCARD>()(args...);
	Functor<TS_CS_BOOKMARK_TITLE>()(args...);
	Functor<TS_CS_BUY_FROM_BOOTH>()(args...);
	Functor<TS_CS_BUY_ITEM>()(args...);
	Functor<TS_CS_CANCEL_ACTION>()(args...);
	Functor<TS_CS_CHANGE_ALIAS>()(args...);
	Functor<TS_CS_CHANGE_ITEM_POSITION>()(args...);
	Functor<TS_CS_CHANGE_LOCATION>()(args...);
	Functor<TS_CS_CHANGE_SUMMON_NAME>()(args...);
	Functor<TS_CS_CHARACTER_LIST>()(args...);
	Functor<TS_CS_CHAT_REQUEST>()(args...);
	Functor<TS_CS_CHECK_BOOTH_STARTABLE>()(args...);
	Functor<TS_CS_CHECK_CHARACTER_NAME>()(args...);
	Functor<TS_CS_CHECK_ILLEGAL_USER>()(args...);
	Functor<TS_CS_COMPETE_ANSWER>()(args...);
	Functor<TS_CS_COMPETE_REQUEST>()(args...);
	Functor<TS_CS_CONTACT>()(args...);
	Functor<TS_CS_CREATE_CHARACTER>()(args...);
	Functor<TS_CS_DECOMPOSE>()(args...);
	Functor<TS_CS_DELETE_CHARACTER>()(args...);
	Functor<TS_CS_DIALOG>()(args...);
	Functor<TS_CS_DONATE_ITEM>()(args...);
	Functor<TS_CS_DROP_ITEM>()(args...);
	Functor<TS_CS_DROP_QUEST>()(args...);
	Functor<TS_CS_EMOTION>()(args...);
	Functor<TS_CS_END_QUEST>()(args...);
	Functor<TS_CS_ENTER_EVENT_AREA>()(args...);
	Functor<TS_CS_ERASE_ITEM>()(args...);
	Functor<TS_CS_FOSTER_CREATURE>()(args...);
	Functor<TS_CS_GAME_GUARD_AUTH_ANSWER>()(args...);
	Functor<TS_CS_GAME_TIME>()(args...);
	Functor<TS_CS_GET_BOOTHS_NAME>()(args...);
	Functor<TS_CS_GET_REGION_INFO>()(args...);
	Functor<TS_CS_GET_SUMMON_SETUP_INFO>()(args...);
	Functor<TS_CS_GET_WEATHER_INFO>()(args...);
	Functor<TS_CS_GROUP_FINDER_LIST>()(args...);
	Functor<TS_CS_HIDE_EQUIP_INFO>()(args...);
	Functor<TS_CS_HUNTAHOLIC_BEGIN_HUNTING>()(args...);
	Functor<TS_CS_HUNTAHOLIC_CREATE_INSTANCE>()(args...);
	Functor<TS_CS_HUNTAHOLIC_INSTANCE_LIST>()(args...);
	Functor<TS_CS_HUNTAHOLIC_JOIN_INSTANCE>()(args...);
	Functor<TS_CS_HUNTAHOLIC_LEAVE_INSTANCE>()(args...);
	Functor<TS_CS_HUNTAHOLIC_LEAVE_LOBBY>()(args...);
	Functor<TS_CS_INSTANCE_GAME_ENTER>()(args...);
	Functor<TS_CS_INSTANCE_GAME_EXIT>()(args...);
	Functor<TS_CS_INSTANCE_GAME_SCORE_REQUEST>()(args...);
	Functor<TS_CS_ITEM_KEEPING_LIST>()(args...);
	Functor<TS_CS_ITEM_KEEPING_TAKE>()(args...);
	Functor<TS_CS_JOB_LEVEL_UP>()(args...);
	Functor<TS_CS_LEARN_SKILL>()(args...);
	Functor<TS_CS_LEAVE_EVENT_AREA>()(args...);
	Functor<TS_CS_LOGIN>()(args...);
	Functor<TS_CS_LOGOUT>()(args...);
	Functor<TS_CS_MIX>()(args...);
	Functor<TS_CS_MONSTER_RECOGNIZE>()(args...);
	Functor<TS_CS_MOVE_REQUEST>()(args...);
	Functor<TS_CS_NURSE_CREATURE>()(args...);
	Functor<TS_CS_OPEN_ITEM_SHOP>()(args...);
	Functor<TS_CS_PUTOFF_CARD>()(args...);
	Functor<TS_CS_PUTOFF_ITEM>()(args...);
	Functor<TS_CS_PUTON_CARD>()(args...);
	Functor<TS_CS_PUTON_ITEM>()(args...);
	Functor<TS_CS_PUTON_ITEM_SET>()(args...);
	Functor<TS_CS_QUERY>()(args...);
	Functor<TS_CS_QUEST_INFO>()(args...);
	Functor<TS_CS_RANKING_TOP_RECORD>()(args...);
	Functor<TS_CS_REGION_UPDATE>()(args...);
	Functor<TS_CS_REPAIR_SOULSTONE>()(args...);
	Functor<TS_CS_REPORT>()(args...);
	Functor<TS_CS_REQUEST_FARM_INFO>()(args...);
	Functor<TS_CS_REQUEST_FARM_MARKET>()(args...);
	Functor<TS_CS_REQUEST_LOGOUT>()(args...);
	Functor<TS_CS_REQUEST_REMOVE_STATE>()(args...);
	Functor<TS_CS_REQUEST_RETURN_LOBBY>()(args...);
	Functor<TS_CS_RESURRECTION>()(args...);
	Functor<TS_CS_RETRIEVE_CREATURE>()(args...);
	Functor<TS_CS_RETURN_LOBBY>()(args...);
	Functor<TS_CS_SECURITY_NO>()(args...);
	Functor<TS_CS_SELL_ITEM>()(args...);
	Functor<TS_CS_SELL_TO_BOOTH>()(args...);
	Functor<TS_CS_SET_MAIN_TITLE>()(args...);
	Functor<TS_CS_SET_PET_NAME>()(args...);
	Functor<TS_CS_SET_PROPERTY>()(args...);
	Functor<TS_CS_SET_SUB_TITLE>()(args...);
	Functor<TS_CS_SKILL>()(args...);
	Functor<TS_CS_SOULSTONE_CRAFT>()(args...);
	Functor<TS_CS_START_BOOTH>()(args...);
	Functor<TS_CS_STOP_BOOTH>()(args...);
	Functor<TS_CS_STOP_WATCH_BOOTH>()(args...);
	Functor<TS_CS_STORAGE>()(args...);
	Functor<TS_CS_SUMMON_CARD_SKILL_LIST>()(args...);
	Functor<TS_CS_SUMMON>()(args...);
	Functor<TS_CS_SWAP_EQUIP>()(args...);
	Functor<TS_CS_TAKE_ITEM>()(args...);
	Functor<TS_CS_TAKEOUT_COMMERCIAL_ITEM>()(args...);
	Functor<TS_CS_TARGETING>()(args...);
	Functor<TS_CS_TRANSMIT_ETHEREAL_DURABILITY>()(args...);
	Functor<TS_CS_TRANSMIT_ETHEREAL_DURABILITY_TO_EQUIPMENT>()(args...);
	Functor<TS_CS_TURN_OFF_PK_MODE>()(args...);
	Functor<TS_CS_TURN_ON_PK_MODE>()(args...);
	Functor<TS_CS_UNBIND_SKILLCARD>()(args...);
	Functor<TS_CS_UPDATE>()(args...);
	Functor<TS_CS_USE_ITEM>()(args...);
	Functor<TS_CS_VERSION>()(args...);
	Functor<TS_CS_WATCH_BOOTH>()(args...);
	Functor<TS_CS_XTRAP_CHECK>()(args...);
	Functor<TS_EQUIP_SUMMON>()(args...);
	Functor<TS_SC_ACHIEVE_TITLE>()(args...);
	Functor<TS_SC_ADDED_SKILL_LIST>()(args...);
	Functor<TS_SC_ADD_PET_INFO>()(args...);
	Functor<TS_SC_ADD_SUMMON_INFO>()(args...);
	Functor<TS_SC_ANTI_HACK>()(args...);
	Functor<TS_SC_ATTACK_EVENT>()(args...);
	Functor<TS_SC_AUCTION_BIDDED_LIST>()(args...);
	Functor<TS_SC_AUCTION_SEARCH>()(args...);
	Functor<TS_SC_AUCTION_SELLING_LIST>()(args...);
	Functor<TS_SC_AURA>()(args...);
	Functor<TS_SC_BATTLE_ARENA_ABSENCE_CHECK>()(args...);
	Functor<TS_SC_BATTLE_ARENA_BATTLE_INFO>()(args...);
	Functor<TS_SC_BATTLE_ARENA_BATTLE_SCORE>()(args...);
	Functor<TS_SC_BATTLE_ARENA_BATTLE_STATUS>()(args...);
	Functor<TS_SC_BATTLE_ARENA_DISCONNECT_BATTLE>()(args...);
	Functor<TS_SC_BATTLE_ARENA_EXERCISE_READY_STATUS>()(args...);
	Functor<TS_SC_BATTLE_ARENA_JOIN_BATTLE>()(args...);
	Functor<TS_SC_BATTLE_ARENA_JOIN_QUEUE>()(args...);
	Functor<TS_SC_BATTLE_ARENA_LEAVE>()(args...);
	Functor<TS_SC_BATTLE_ARENA_PENALTY_INFO>()(args...);
	Functor<TS_SC_BATTLE_ARENA_RECONNECT_BATTLE>()(args...);
	Functor<TS_SC_BATTLE_ARENA_RESULT>()(args...);
	Functor<TS_SC_BATTLE_ARENA_UPDATE_WAIT_USER_COUNT>()(args...);
	Functor<TS_SC_BELT_SLOT_INFO>()(args...);
	Functor<TS_SC_BONUS_EXP_JP>()(args...);
	Functor<TS_SC_BOOKMARK_TITLE>()(args...);
	Functor<TS_SC_BOOTH_CLOSED>()(args...);
	Functor<TS_SC_BOOTH_TRADE_INFO>()(args...);
	Functor<TS_SC_CANT_ATTACK>()(args...);
	Functor<TS_SC_CHANGE_LOCATION>()(args...);
	Functor<TS_SC_CHANGE_NAME>()(args...);
	Functor<TS_SC_CHANGE_TITLE_CONDITION>()(args...);
	Functor<TS_SC_CHARACTER_LIST>()(args...);
	Functor<TS_SC_CHAT>()(args...);
	Functor<TS_SC_CHAT_LOCAL>()(args...);
	Functor<TS_SC_CHAT_RESULT>()(args...);
	Functor<TS_SC_COMMERCIAL_STORAGE_INFO>()(args...);
	Functor<TS_SC_COMMERCIAL_STORAGE_LIST>()(args...);
	Functor<TS_SC_COMPETE_ANSWER>()(args...);
	Functor<TS_SC_COMPETE_COUNTDOWN>()(args...);
	Functor<TS_SC_COMPETE_END>()(args...);
	Functor<TS_SC_COMPETE_REQUEST>()(args...);
	Functor<TS_SC_COMPETE_START>()(args...);
	Functor<TS_SC_DECOMPOSE_RESULT>()(args...);
	Functor<TS_SC_DESTROY_ITEM>()(args...);
	Functor<TS_SC_DETECT_RANGE_UPDATE>()(args...);
	Functor<TS_SC_DIALOG>()(args...);
	Functor<TS_SC_DISCONNECT_DESC>()(args...);
	Functor<TS_SC_DROP_RESULT>()(args...);
	Functor<TS_SC_EMOTION>()(args...);
	Functor<TS_SC_ENERGY>()(args...);
	Functor<TS_SC_ENTER>()(args...);
	Functor<TS_SC_ERASE_ITEM>()(args...);
	Functor<TS_SC_EXP_UPDATE>()(args...);
	Functor<TS_SC_FARM_INFO>()(args...);
	Functor<TS_SC_GAME_GUARD_AUTH_QUERY>()(args...);
	Functor<TS_SC_GAME_TIME>()(args...);
	Functor<TS_SC_GENERAL_MESSAGE_BOX>()(args...);
	Functor<TS_SC_GET_BOOTHS_NAME>()(args...);
	Functor<TS_SC_GET_CHAOS>()(args...);
	Functor<TS_SC_GOLD_UPDATE>()(args...);
	Functor<TS_SC_GROUP_FINDER_DETAIL>()(args...);
	Functor<TS_SC_GROUP_FINDER_LIST>()(args...);
	Functor<TS_SC_HAIR_INFO>()(args...);
	Functor<TS_SC_HIDE_EQUIP_INFO>()(args...);
	Functor<TS_SC_HPMP>()(args...);
	Functor<TS_SC_HUNTAHOLIC_BEGIN_COUNTDOWN>()(args...);
	Functor<TS_SC_HUNTAHOLIC_BEGIN_HUNTING>()(args...);
	Functor<TS_SC_HUNTAHOLIC_HUNTING_SCORE>()(args...);
	Functor<TS_SC_HUNTAHOLIC_INSTANCE_INFO>()(args...);
	Functor<TS_SC_HUNTAHOLIC_INSTANCE_LIST>()(args...);
	Functor<TS_SC_HUNTAHOLIC_MAX_POINT_ACHIEVED>()(args...);
	Functor<TS_SC_HUNTAHOLIC_UPDATE_SCORE>()(args...);
	Functor<TS_SC_INSTANCE_GAME_SCORE_REQUEST>()(args...);
	Functor<TS_SC_INVENTORY>()(args...);
	Functor<TS_SC_ITEM_COOL_TIME>()(args...);
	Functor<TS_SC_ITEM_DROP_INFO>()(args...);
	Functor<TS_SC_ITEM_KEEPING_LIST>()(args...);
	Functor<TS_SC_ITEM_WEAR_INFO>()(args...);
	Functor<TS_SC_LEAVE>()(args...);
	Functor<TS_SC_LEVEL_UPDATE>()(args...);
	Functor<TS_SC_LOGIN_RESULT>()(args...);
	Functor<TS_SC_MARKET>()(args...);
	Functor<TS_SC_MIX_RESULT>()(args...);
	Functor<TS_SC_MOUNT_SUMMON>()(args...);
	Functor<TS_SC_MOVE_ACK>()(args...);
	Functor<TS_SC_MOVE>()(args...);
	Functor<TS_SC_NPC_TRADE_INFO>()(args...);
	Functor<TS_SC_OPEN_GUILD_WINDOW>()(args...);
	Functor<TS_SC_OPEN_ITEM_SHOP>()(args...);
	Functor<TS_SC_OPEN_STORAGE>()(args...);
	Functor<TS_SC_OPEN_TITLE>()(args...);
	Functor<TS_SC_OPEN_URL>()(args...);
	Functor<TS_SC_PROPERTY>()(args...);
	Functor<TS_SC_QUEST_INFOMATION>()(args...);
	Functor<TS_SC_QUEST_LIST>()(args...);
	Functor<TS_SC_QUEST_STATUS>()(args...);
	Functor<TS_SC_RANKING_TOP_RECORD>()(args...);
	Functor<TS_SC_REGEN_HPMP>()(args...);
	Functor<TS_SC_REGEN_INFO>()(args...);
	Functor<TS_SC_REGION_ACK>()(args...);
	Functor<TS_SC_REMAIN_TITLE_TIME>()(args...);
	Functor<TS_SC_REMOVE_PET_INFO>()(args...);
	Functor<TS_SC_REMOVE_SUMMON_INFO>()(args...);
	Functor<TS_SC_REQUEST_SECURITY_NO>()(args...);
	Functor<TS_SC_RESULT_FOSTER>()(args...);
	Functor<TS_SC_RESULT>()(args...);
	Functor<TS_SC_RESULT_NURSE>()(args...);
	Functor<TS_SC_RESULT_RETRIEVE>()(args...);
	Functor<TS_SC_SET_MAIN_TITLE>()(args...);
	Functor<TS_SC_SET_SUB_TITLE>()(args...);
	Functor<TS_SC_SET_TIME>()(args...);
	Functor<TS_SC_SHOW_CREATE_ALLIANCE>()(args...);
	Functor<TS_SC_SHOW_CREATE_GUILD>()(args...);
	Functor<TS_SC_SHOW_SET_PET_NAME>()(args...);
	Functor<TS_SC_SHOW_SOULSTONE_REPAIR_WINDOW>()(args...);
	Functor<TS_SC_SHOW_SUMMON_NAME_CHANGE>()(args...);
	Functor<TS_SC_SHOW_WINDOW>()(args...);
	Functor<TS_SC_SKILLCARD_INFO>()(args...);
	Functor<TS_SC_SKILL>()(args...);
	Functor<TS_SC_SKILL_LEVEL_LIST>()(args...);
	Functor<TS_SC_SKILL_LIST>()(args...);
	Functor<TS_SC_SP>()(args...);
	Functor<TS_SC_STATE>()(args...);
	Functor<TS_SC_STATE_RESULT>()(args...);
	Functor<TS_SC_STAT_INFO>()(args...);
	Functor<TS_SC_STATUS_CHANGE>()(args...);
	Functor<TS_SC_SUMMON_EVOLUTION>()(args...);
	Functor<TS_SC_TAKE_ITEM_RESULT>()(args...);
	Functor<TS_SC_TAMING_INFO>()(args...);
	Functor<TS_SC_TARGET>()(args...);
	Functor<TS_SC_TITLE_CONDITION_LIST>()(args...);
	Functor<TS_SC_TITLE_LIST>()(args...);
	Functor<TS_SC_UNMOUNT_SUMMON>()(args...);
	Functor<TS_SC_UNSUMMON>()(args...);
	Functor<TS_SC_UNSUMMON_NOTICE>()(args...);
	Functor<TS_SC_UNSUMMON_PET>()(args...);
	Functor<TS_SC_UPDATE_GUILD_ICON>()(args...);
	Functor<TS_SC_UPDATE_ITEM_COUNT>()(args...);
	Functor<TS_SC_URL_LIST>()(args...);
	Functor<TS_SC_USE_ITEM_RESULT>()(args...);
	Functor<TS_SC_WARP>()(args...);
	Functor<TS_SC_WATCH_BOOTH>()(args...);
	Functor<TS_SC_WEAR_INFO>()(args...);
	Functor<TS_SC_WEATHER_INFO>()(args...);
	Functor<TS_SC_XTRAP_CHECK>()(args...);
	Functor<TS_TIMESYNC>()(args...);
	Functor<TS_TRADE>()(args...);

	Functor<TS_SC_SHOW_SOULSTONE_CRAFT_WINDOW>()(args...);
	Functor<TS_CS_DONATE_REWARD>()(args...);
}

#endif
