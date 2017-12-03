#include "PacketFilter.h"
#include "Core/PrintfFormats.h"
#include "Core/Utils.h"
#include "GameClient/TS_SC_CHAT.h"
#include "Packet/JSONWriter.h"
#include <sstream>

PacketFilter::PacketFilter(IFilterEndpoint* client, IFilterEndpoint* server, PacketFilter* oldFilter)
    : IFilter(client, server) {
	if(oldFilter) {
		data = oldFilter->data;
		oldFilter->data = nullptr;
		if(data->file)
			fclose(data->file);
		data->file = nullptr;
	} else {
		data = new Data;
	}
}

PacketFilter::~PacketFilter() {
	if(data)
		delete data;
}

bool PacketFilter::onServerPacket(const TS_MESSAGE* packet, ServerType serverType) {
	clientp = client;
	serverVersion = server->getPacketVersion();

	if(serverType != ST_Game)
		return true;

	switch(packet->id) {
		case_packet_is(TS_SC_LOGIN_RESULT)
		    packet->process(this, &PacketFilter::onLoginResultMessage, server->getPacketVersion());
		break;

		case_packet_is(TS_SC_ENTER) packet->process(this, &PacketFilter::onEnterMessage, server->getPacketVersion());
		break;

		case TS_SC_ATTACK_EVENT::packetID:
			packet->process(this, &PacketFilter::onAttackEventMessage, server->getPacketVersion());
			break;

		case TS_SC_SKILL::packetID:
			packet->process(this, &PacketFilter::onSkillMessage, server->getPacketVersion());
			break;

		case TS_SC_STATE_RESULT::packetID:
			packet->process(this, &PacketFilter::onStateResultMessage, server->getPacketVersion());
			break;
	}
	return true;
}

void PacketFilter::onLoginResultMessage(const TS_SC_LOGIN_RESULT* packet) {
	data->players[packet->handle] = packet->name;
}

void PacketFilter::onEnterMessage(const TS_SC_ENTER* packet) {
	if(packet->objType == EOT_Player) {
		data->players[packet->handle] = packet->playerInfo.szName;
	} else if(packet->objType == EOT_Summon) {
		std::string name = packet->summonInfo.szName;

		if(data->players.count(packet->summonInfo.master_handle)) {
			const std::string& masterName = data->players[packet->summonInfo.master_handle];
			name += " (Pet of " + masterName + ")";
		}

		data->players[packet->handle] = name;
	} else if(packet->objType == EOT_Monster) {
		data->players[packet->handle] = "#mob|" + std::to_string(uint32_t(packet->monsterInfo.monster_id));
	}
}

void PacketFilter::onAttackEventMessage(const TS_SC_ATTACK_EVENT* packet) {
	SwingData swingData;

	showPacketJson(packet, serverVersion);

	swingData.attacker = getHandleName(packet->attacker_handle);
	swingData.victim = getHandleName(packet->target_handle);
	swingData.attackName = "Attack";
	swingData.time = Utils::getTimeInMsec();

	for(const ATTACK_INFO& attackInfo : packet->attack) {
		int hitType = 0;
		if(attackInfo.flag & AIF_PerfectBlock)
			hitType |= DT_PerfectBlock;
		if(attackInfo.flag & AIF_Block)
			hitType |= DT_Block;
		if(attackInfo.flag & AIF_Miss)
			hitType |= DT_Miss;
		swingData.damageType = hitType;

		if(attackInfo.flag & AIF_Critical)
			swingData.critical = true;
		else
			swingData.critical = false;

		if(attackInfo.damage != 0 || swingData.damageType != 0) {
			if(attackInfo.damage >= 0)
				swingData.swingType = SwingType::Melee;
			else
				swingData.swingType = SwingType::Healing;

			swingData.damage = attackInfo.damage;
			swingData.damageElement = "Physical";
			sendCombatLogLine(swingData);
		}

		if(attackInfo.mp_damage) {
			swingData.swingType = SwingType::ManaDrain;
			swingData.damage = attackInfo.mp_damage;
			swingData.damageElement = "Physical";
			sendCombatLogLine(swingData);
		}
	}
}

void PacketFilter::onSkillMessage(const TS_SC_SKILL* packet) {
	static const char* ELEMENTAL_TYPE[] = {"None", "Fire", "Water", "Wind", "Earth", "Light", "Dark"};
	SwingData swingData;

	showPacketJson(packet, serverVersion);

	if(packet->type != ST_Fire && packet->type != ST_RegionFire)
		return;

	swingData.attacker = getHandleName(packet->caster);
	swingData.attackName = "#skill|" + std::to_string(packet->skill_id) + "|" + std::to_string(packet->skill_level);
	swingData.time = Utils::getTimeInMsec();

	for(const TS_SC_SKILL__HIT_DETAILS& hit : packet->fire.hits) {
		swingData.victim = getHandleName(hit.hTarget);

		if(hit.type == SHT_DAMAGE || hit.type == SHT_MAGIC_DAMAGE || hit.type == SHT_DAMAGE_WITH_KNOCK_BACK ||
		   hit.type == SHT_CHAIN_DAMAGE || hit.type == SHT_CHAIN_MAGIC_DAMAGE) {
			const TS_SC_SKILL__HIT_DAMAGE_INFO* damageInfo;
			if(hit.type == SHT_DAMAGE || hit.type == SHT_MAGIC_DAMAGE)
				damageInfo = &hit.hitDamage.damage;
			else if(hit.type == SHT_DAMAGE_WITH_KNOCK_BACK)
				damageInfo = &hit.hitDamageWithKnockBack.damage;
			else
				damageInfo = &hit.hitChainDamage.damage;

			swingData.damageType = 0;

			if(damageInfo->flag & 1)
				swingData.critical = true;
			else
				swingData.critical = false;

			swingData.swingType = SwingType::NonMelee;

			if(damageInfo->damage) {
				swingData.damage = damageInfo->damage;
				swingData.damageElement = ELEMENTAL_TYPE[damageInfo->damage_type];
				sendCombatLogLine(swingData);
			}
		} else if(hit.type == SHT_ADD_HP || hit.type == SHT_ADD_MP) {
			const TS_SC_SKILL__HIT_ADD_STAT& hitAddStat = hit.hitAddStat;

			swingData.damageType = 0;
			swingData.critical = false;

			if(hit.type == SHT_ADD_HP) {
				if(hitAddStat.nIncStat >= 0)
					swingData.swingType = SwingType::Healing;
				else
					swingData.swingType = SwingType::NonMelee;
			} else {
				if(hitAddStat.nIncStat >= 0)
					swingData.swingType = SwingType::ManaHealing;
				else
					swingData.swingType = SwingType::ManaDrain;
			}

			swingData.damage = hitAddStat.nIncStat;
			swingData.damageElement = "None";
			sendCombatLogLine(swingData);
		} else if(hit.type == SHT_ADD_HP_MP_SP) {
			const TS_SC_SKILL__HIT_ADDHPMPSP& hitAddHPMPSP = hit.hitAddHPMPSP;

			swingData.damageType = 0;
			swingData.critical = false;

			swingData.damageElement = "None";

			if(hitAddHPMPSP.nIncHP) {
				swingData.damage = hitAddHPMPSP.nIncHP;

				if(hitAddHPMPSP.nIncHP >= 0)
					swingData.swingType = SwingType::Healing;
				else
					swingData.swingType = SwingType::NonMelee;

				sendCombatLogLine(swingData);
			}
			if(hitAddHPMPSP.nIncMP) {
				swingData.damage = hitAddHPMPSP.nIncMP;

				if(hitAddHPMPSP.nIncMP >= 0)
					swingData.swingType = SwingType::ManaHealing;
				else
					swingData.swingType = SwingType::ManaDrain;

				sendCombatLogLine(swingData);
			}
		} else if(hit.type == SHT_CHAIN_HEAL) {
			const TS_SC_SKILL__HIT_CHAIN_HEAL& hitChainHeal = hit.hitChainHeal;

			swingData.damageType = 0;
			swingData.critical = false;

			swingData.damageElement = "None";

			swingData.damage = hitChainHeal.nIncHP;

			if(hitChainHeal.nIncHP >= 0)
				swingData.swingType = SwingType::Healing;
			else
				swingData.swingType = SwingType::NonMelee;

			sendCombatLogLine(swingData);
		} else if(hit.type == SHT_REBIRTH) {
			const TS_SC_SKILL__HIT_REBIRTH& hitRebirth = hit.hitRebirth;

			swingData.damageType = 0;
			swingData.critical = false;

			swingData.damageElement = "None";

			if(hitRebirth.nIncHP) {
				swingData.damage = hitRebirth.nIncHP;
				swingData.swingType = SwingType::Healing;
				sendCombatLogLine(swingData);
			}
			if(hitRebirth.nIncMP) {
				swingData.damage = hitRebirth.nIncMP;
				swingData.swingType = SwingType::ManaHealing;
				sendCombatLogLine(swingData);
			}
		} else if(hit.type == SHT_RESULT) {
			const TS_SC_SKILL__HIT_RESULT& hitResult = hit.hitResult;

			if(hitResult.bResult == false) {
				swingData.damageType = DT_Miss;
				swingData.critical = false;

				swingData.damageElement = "None";

				swingData.damage = 0;
				swingData.swingType = SwingType::NonMelee;
				sendCombatLogLine(swingData);
			}
		}
	}
}

void PacketFilter::onStateResultMessage(const TS_SC_STATE_RESULT* packet) {
	SwingData swingData;

	showPacketJson(packet, serverVersion);

	swingData.attacker = getHandleName(packet->caster_handle);
	swingData.victim = getHandleName(packet->target_handle);
	swingData.attackName = "#dot|" + std::to_string(packet->code) + "|" + std::to_string(packet->level);
	swingData.time = Utils::getTimeInMsec();

	swingData.damageType = 0;
	swingData.critical = false;
	swingData.damage = packet->value;
	swingData.damageElement = "None";

	switch(packet->result_type) {
		case SRT_Damage:
			if(swingData.damage >= 0)
				swingData.swingType = SwingType::NonMelee;
			else
				swingData.swingType = SwingType::Healing;
			break;

		case SRT_MagicDamage:
			if(swingData.damage >= 0)
				swingData.swingType = SwingType::ManaDrain;
			else
				swingData.swingType = SwingType::ManaHealing;
			break;

		case SRT_Heal:
			if(swingData.damage >= 0)
				swingData.swingType = SwingType::Healing;
			else
				swingData.swingType = SwingType::NonMelee;
			break;

		case SRT_MagicHeal:
			if(swingData.damage >= 0)
				swingData.swingType = SwingType::ManaHealing;
			else
				swingData.swingType = SwingType::ManaDrain;
			break;

		default:
			return;
	}

	sendCombatLogLine(swingData);
}

std::string PacketFilter::getHandleName(uint32_t handle) {
	if(data->players.count(handle)) {
		return data->players[handle];
	} else {
		return "#handle|" + std::to_string(handle);
	}
}

void PacketFilter::sendCombatLogLine(const PacketFilter::SwingData& data) {
	TS_SC_CHAT chatRqst;
	char buffer[256];

	if(!clientp)
		return;

	if(!this->data->file) {
		this->data->file = fopen("combatlog.txt", "at");
	}

	std::string swingTypeStr = "unkST";
	switch(data.swingType) {
		case SwingType::Melee:
			swingTypeStr = "Melee";
			break;
		case SwingType::NonMelee:
			swingTypeStr = "NonMelee";
			break;
		case SwingType::Healing:
			swingTypeStr = "Healing";
			break;
		case SwingType::ManaDrain:
			swingTypeStr = "ManaDrain";
			break;
		case SwingType::ManaHealing:
			swingTypeStr = "ManaHealing";
			break;
	}

	std::string damageTypeStr;
	if(data.damageType & DT_Miss) {
		damageTypeStr = "Miss";
	} else if(data.damageType & DT_PerfectBlock) {
		damageTypeStr = "PerfectBlock";
	} else if(data.damageType & DT_Block) {
		damageTypeStr = "Block";
	}

	if(this->data->file) {
		fprintf(this->data->file,
		        "%" PRIu64 "\t%s\t%s\t%s\t%s\t%s\t%" PRId64 "\t%s\t%s\n",
		        data.time,
		        data.attacker.c_str(),
		        data.victim.c_str(),
		        data.attackName.c_str(),
		        swingTypeStr.c_str(),
		        damageTypeStr.c_str(),
		        data.damage,
		        data.critical ? "crit" : "",
		        data.damageElement.c_str());
		fflush(this->data->file);
	}

	sprintf(buffer,
	        "%s->%s: %s(%s%s%s) = %" PRId64 "%s(%s)",
	        data.attacker.c_str(),
	        data.victim.c_str(),
	        data.attackName.c_str(),
	        swingTypeStr.c_str(),
	        damageTypeStr.empty() ? "" : ", ",
	        damageTypeStr.c_str(),
	        data.damage,
	        data.critical ? "!" : "",
	        data.damageElement.c_str());

	Object::logStatic(Object::LL_Info, "rzfilter_combatlog", "Damage: %s\n", buffer);

	chatRqst.message = buffer;
	if(chatRqst.message.size() > 32766)
		chatRqst.message.resize(32766);
	// chatRqst.message.append(1, '\0');

	chatRqst.szSender = "Filter";
	chatRqst.type = CHAT_WHISPER;

	// clientp->sendPacket(chatRqst);
}

template<class Packet> void PacketFilter::showPacketJson(const Packet* packet, int version) {
	return;
	JSONWriter jsonWriter(version, false);
	packet->serialize(&jsonWriter);
	jsonWriter.finalize();
	std::string jsonData = jsonWriter.toString();

	Object::logStatic(Object::LL_Info, "rzfilter_combatlog", "%s packet:\n%s\n", Packet::getName(), jsonData.c_str());
}

IFilter* createFilter(IFilterEndpoint* client, IFilterEndpoint* server, IFilter* oldFilter) {
	Object::logStatic(Object::LL_Info, "rzfilter_combatlog", "Loaded filter from data: %p\n", oldFilter);
	return new PacketFilter(client, server, (PacketFilter*) oldFilter);
}

void destroyFilter(IFilter* filter) {
	delete filter;
}
