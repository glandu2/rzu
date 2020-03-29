#ifndef PACKETFILTER_H
#define PACKETFILTER_H

#include "IFilter.h"
#include "LibGlobal.h"
#include <stdio.h>
#include <unordered_map>

#include "GameClient/TS_SC_ATTACK_EVENT.h"
#include "GameClient/TS_SC_ENTER.h"
#include "GameClient/TS_SC_LOGIN_RESULT.h"
#include "GameClient/TS_SC_SKILL.h"
#include "GameClient/TS_SC_STATE_RESULT.h"

class PacketFilter : public IFilter {
protected:
	enum class SwingType {
		Melee = 0,
		NonMelee = 1,
		Healing = 2,
		ManaDrain = 3,
		ManaHealing = 4,
	};

	enum DamageType { DT_Miss = 1, DT_PerfectBlock = 2, DT_Block = 4 };

	struct SwingData {
		SwingType swingType;
		bool critical;
		std::string attacker;
		std::string victim;
		std::string attackName;
		int64_t damage;
		int damageType;
		std::string damageElement;
		uint64_t time;
	};

public:
	PacketFilter(IFilterEndpoint* client, IFilterEndpoint* server, ServerType serverType, PacketFilter* data);
	~PacketFilter();

	virtual bool onServerPacket(const TS_MESSAGE* packet) override;

protected:
private:
	void onLoginResultMessage(const TS_SC_LOGIN_RESULT* packet);
	void onEnterMessage(const TS_SC_ENTER* packet);
	void onAttackEventMessage(const TS_SC_ATTACK_EVENT* packet);
	void onSkillMessage(const TS_SC_SKILL* packet);
	void onStateResultMessage(const TS_SC_STATE_RESULT* packet);

	std::string getHandleName(ar_handle_t handle);

	void sendCombatLogLine(const SwingData& data);

	template<class Packet> void showPacketJson(const Packet* packet, int version);

private:
	struct Data {
		std::unordered_map<ar_handle_t, std::string> players;
		FILE* file = nullptr;
	};
	Data* data;

	IFilterEndpoint* clientp;
	int serverVersion;
};

extern "C" {
SYMBOL_EXPORT IFilter* createFilter(IFilterEndpoint* client,
                                    IFilterEndpoint* server,
                                    IFilter::ServerType serverType,
                                    IFilter* oldFilter);
SYMBOL_EXPORT void destroyFilter(IFilter* filter);
}

#endif  // PACKETFILTER_H
