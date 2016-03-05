#include "PacketFilter.h"
#include "GameClient/TS_SC_SKILL.h"
#include "GameClient/TS_SC_CHAT.h"
#include <sstream>

PacketFilter::PacketFilter(PacketFilter *oldFilter)
{
	if(oldFilter) {
		data = oldFilter->data;
		oldFilter->data = nullptr;
	} else {
		data = new Data;
	}
}

PacketFilter::~PacketFilter()
{
	if(data)
		delete data;
}

void PacketFilter::sendChatMessage(IFilterEndpoint* client, const char* msg) {
	TS_SC_CHAT* chatRqst;
	size_t msgLen = strlen(msg);
	if(msgLen > 126)
		msgLen = 126;

	chatRqst = TS_MESSAGE_WNA::create<TS_SC_CHAT, char>((int)msgLen+1);

	chatRqst->len = (uint16_t)msgLen;
	strncpy(chatRqst->message, msg, chatRqst->len);
	chatRqst->message[chatRqst->len] = 0;
	strcpy(chatRqst->szSender, "Filter");
	chatRqst->type = 3;

	client->sendPacket(chatRqst);

	TS_MESSAGE_WNA::destroy(chatRqst);
}

bool PacketFilter::onServerPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet) {
	clientp = client;

	switch(packet->id) {
		case TS_SC_SKILL::packetID: {
			/*const TS_SC_SKILL* skillPacket = reinterpret_cast<const TS_SC_SKILL*>(packet);
			char buffer[1024];
			char *p = buffer;
			buffer[0] = 0;

			p += sprintf(p, "Skill %d Lv%d from 0x%08X to 0x%08X at (%d, %d) cost %dhp %dmp ",
						 skillPacket->skill_id,
						 skillPacket->skill_level,
						 skillPacket->caster,
						 skillPacket->target,
						 (int)skillPacket->x,
						 (int)skillPacket->y,
						 skillPacket->hp_cost,
						 skillPacket->mp_cost);

			switch(skillPacket->type) {
				case TS_SC_SKILL::FIRE:
				case TS_SC_SKILL::REGION_FIRE:
					p += sprintf(p, "Fire(multiple %d, range %d, %d targets, %d fires):",
								 skillPacket->fire.bMultiple,
								 (int)skillPacket->fire.range,
								 skillPacket->fire.target_count,
								 skillPacket->fire.fire_count);
					sendChatMessage(client, buffer);
					p = buffer;

					for(int i = 0; i < skillPacket->fire.count; i++) {
						const TS_SC_SKILL::FireType::HitDetails* hit = &skillPacket->fire.hits[i];
						switch(hit->type) {
							case TS_SC_SKILL::FireType::HitDetails::DAMAGE:
							case TS_SC_SKILL::FireType::HitDetails::DAMAGE_WITH_KNOCK_BACK:
							case TS_SC_SKILL::FireType::HitDetails::MAGIC_DAMAGE:
								char damageType[64];
								switch(hit->damage.damage_type) {
									case TS_SC_SKILL::FireType::TYPE_NONE: strcpy(damageType, "none"); break;
									case TS_SC_SKILL::FireType::TYPE_FIRE: strcpy(damageType, "fire"); break;
									case TS_SC_SKILL::FireType::TYPE_WATER: strcpy(damageType, "water"); break;
									case TS_SC_SKILL::FireType::TYPE_WIND: strcpy(damageType, "wind"); break;
									case TS_SC_SKILL::FireType::TYPE_EARTH: strcpy(damageType, "earth"); break;
									case TS_SC_SKILL::FireType::TYPE_LIGHT: strcpy(damageType, "light"); break;
									case TS_SC_SKILL::FireType::TYPE_DARK: strcpy(damageType, "dark"); break;
									case TS_SC_SKILL::FireType::TYPE_COUNT: strcpy(damageType, "count"); break;
									default: strcpy(damageType, "unknown"); break;
								}

								sprintf(buffer, " - Damage(target: 0x%08X, %d hp, %d %s damage, flag %d)",
										hit->damage.hTarget,
										hit->damage.target_hp,
										hit->damage.damage,
										damageType,
										hit->damage.flag);
								sendChatMessage(client, buffer);
								break;
							case TS_SC_SKILL::FireType::HitDetails::ADD_HP:
								sprintf(buffer, " - Heal(target: 0x%08X, %d hp, +%d HP)",
										hit->addHP.hTarget,
										hit->addHP.target_hp,
										hit->addHP.nIncHP);
								sendChatMessage(client, buffer);
								break;
							case TS_SC_SKILL::FireType::HitDetails::RESULT:
								sprintf(buffer, " - Result(target: 0x%08X, result %d: %d)",
										hit->result.hTarget,
										hit->result.bResult,
										hit->result.success_type);
								sendChatMessage(client, buffer);
								break;
							default:
								sprintf(buffer, " - Unknown(%d)(target: 0x%08X)",
										hit->type,
										hit->damage.hTarget);
								sendChatMessage(client, buffer);
						}

					}
					break;
				case TS_SC_SKILL::CANCEL:
					p += sprintf(p, "Cancel");
					sendChatMessage(client, buffer);
					break;

				case TS_SC_SKILL::COMPLETE:
					p += sprintf(p, "Complete");
					sendChatMessage(client, buffer);
					break;

				case TS_SC_SKILL::CASTING:
					p += sprintf(p, "Casting");
					sendChatMessage(client, buffer);
					break;

				case TS_SC_SKILL::CASTING_UPDATE:
					p += sprintf(p, "Casting update");
					sendChatMessage(client, buffer);
					break;

				default:
					p += sprintf(p, "unknown (%d)", skillPacket->type);
					sendChatMessage(client, buffer);
			}*/
			break;
		}

		case TS_SC_STATE_RESULT::packetID: {
			const TS_SC_STATE_RESULT* stateResult = reinterpret_cast<const TS_SC_STATE_RESULT*>(packet);
			char buffer[1024];

			sprintf(buffer, "DOT(caster 0x%08X, target 0x%08X, id %d Lv%d, result %d, value %d, targetval %d, total %d, final %d)",
					stateResult->caster_handle,
					stateResult->target_handle,
					stateResult->code,
					stateResult->level,
					stateResult->result_type,
					stateResult->value,
					stateResult->target_value,
					stateResult->total_amount,
					stateResult->final);
			sendChatMessage(client, buffer);
			break;
		}

		case TS_SC_INVENTORY::packetID:
			packet->process(this, &PacketFilter::onInventory, server->getPacketVersion());
			break;

		case TS_SC_ATTACK_EVENT::packetID:
			packet->process(this, &PacketFilter::onAttack, server->getPacketVersion());
			break;
	}

	return true;
}

bool PacketFilter::onClientPacket(IFilterEndpoint* client, IFilterEndpoint* server, const TS_MESSAGE* packet) {
	return true;
}

void PacketFilter::onInventory(const TS_SC_INVENTORY *packet) {
	int i;
	for(i = 0; i < packet->items.size(); i++) {
		const TS_ITEM_INFO& inventoryItem = packet->items[i];
		Item item;
		item.handle = inventoryItem.base_info.handle;
		item.code = inventoryItem.base_info.code;
		item.uid = inventoryItem.base_info.uid;
		item.count = inventoryItem.base_info.count;
		data->items.insert(std::pair<uint32_t, Item>(item.handle, item));
	}
}

void PacketFilter::onAttack(const TS_SC_ATTACK_EVENT *packet) {
/*	char buffer[512];

	sprintf(buffer,
			"Attack evt: 0x%08X > 0x%08X, %dspd, %ddl, act: %d, flg: %d",
			packet->attacker_handle,
			packet->target_handle,
			packet->attack_speed,
			packet->attack_delay,
			packet->attack_action,
			packet->attack_flag);
	sendChatMessage(clientp, buffer);

	for(int i = 0; i < packet->attack.size(); i++) {
		const ATTACK_INFO& attackInfo = packet->attack[i];

		sprintf(buffer,
				" - Atk: %dhp, %dmp, flg: %d", attackInfo.damage, attackInfo.mp_damage,
				attackInfo.flag);
		sendChatMessage(clientp, buffer);

		sprintf(buffer,
				"   - %d, %d, %d, %d, %d, %d, %d",
				attackInfo.elemental_damage[0],
				attackInfo.elemental_damage[1],
				attackInfo.elemental_damage[2],
				attackInfo.elemental_damage[3],
				attackInfo.elemental_damage[4],
				attackInfo.elemental_damage[5],
				attackInfo.elemental_damage[6]);
		sendChatMessage(clientp, buffer);

		sprintf(buffer,
				"   - tgt: %dhp, %dmp",
				attackInfo.target_hp,
				attackInfo.target_mp);
		sendChatMessage(clientp, buffer);
	}*/
}

IFilter *createFilter(IFilter *oldFilter)
{
	Object::logStatic(Object::LL_Info, "rzfilter_module", "Loaded filter from data: %p\n", oldFilter);
	return new PacketFilter((PacketFilter*)oldFilter);
}

void destroyFilter(IFilter *filter)
{
	delete filter;
}
