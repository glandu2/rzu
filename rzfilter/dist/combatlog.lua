-- rzfilter lua module to output a combatlog file
-- To be used to parse DPS and heal/s data with Advanced Combat Tracker tool
-- See https://advancedcombattracker.com/

local json = require("json")
local utils = require("utils")

local playerNameByHandle = {} -- player handle / name map

local EOT_Player = 0
local EOT_NPC = 1
local EOT_Item = 2
local EOT_Monster = 3
local EOT_Summon = 4
local EOT_Skill = 5
local EOT_FieldProp = 6
local EOT_Pet = 7

local ST_Fire = 0
local ST_Casting = 1
local ST_CastingUpdate = 2
local ST_Cancel = 3
local ST_RegionFire = 4
local ST_Complete = 5

local SHT_DAMAGE = 0
local SHT_MAGIC_DAMAGE = 1
local SHT_DAMAGE_WITH_KNOCK_BACK = 2
local SHT_RESULT = 10
local SHT_ADD_HP = 20
local SHT_ADD_MP = 21
local SHT_ADD_HP_MP_SP = 22
local SHT_REBIRTH = 23
local SHT_RUSH = 30
local SHT_CHAIN_DAMAGE = 40
local SHT_CHAIN_MAGIC_DAMAGE = 41
local SHT_CHAIN_HEAL = 42
local SHT_NOT_USE = 100

local SDT_TYPE_NONE = 0
local SDT_TYPE_FIRE = 1
local SDT_TYPE_WATER = 2
local SDT_TYPE_WIND = 3
local SDT_TYPE_EARTH = 4
local SDT_TYPE_LIGHT = 5
local SDT_TYPE_DARK = 6
local SDT_TYPE_COUNT = 7

local SRST_AddState = 10 
local SRST_RemoveState = 11
local SRST_AddHate = 12
local SRST_TurnOn = 21
local SRST_TurnOff = 22
local SRST_SummonDead = 30
local SRST_TargetDead = 31
local SRST_CreateItem = 40
local SRST_RespawnMonster = 41

local AIF_PerfectBlock = 1
local AIF_Block = 2
local AIF_Miss = 4
local AIF_Critical = 8

local SwingType_Melee = 0
local SwingType_NonMelee = 1
local SwingType_Healing = 2
local SwingType_ManaDrain = 3
local SwingType_ManaHealing = 4

local DT_Miss = 1
local DT_PerfectBlock = 2
local DT_Block = 4

local SRT_Damage = 1
local SRT_MagicDamage = 2
local SRT_SPDamage = 3
local SRT_Heal = 4
local SRT_MagicHeal = 5
local SRT_SPHeal = 6


local function showPacketJson(packet)
	--print(json.encode(packet))
end

local function getHandleName(handle)
	return playerNameByHandle[handle] or ("#handle|" .. handle)
end

local outputFile = nil
local function sendCombatLogLine(swingData)
	if not outputFile then
		outputFile = io.open("combatlog.txt", "a")
	end

	local swingTypeStr
	if swingData.swingType == SwingType_Melee then
		swingTypeStr = "Melee"
	elseif swingData.swingType == SwingType_NonMelee then
		swingTypeStr = "NonMelee"
	elseif swingData.swingType == SwingType_Healing then
		swingTypeStr = "Healing"
	elseif swingData.swingType == SwingType_ManaDrain then
		swingTypeStr = "ManaDrain"
	elseif swingData.swingType == SwingType_ManaHealing then
		swingTypeStr = "ManaHealing"
	else
		swingTypeStr = "unkST"
	end

	local damageTypeStr;
	if (swingData.damageType & DT_Miss) ~= 0 then
		damageTypeStr = "Miss"
	elseif (swingData.damageType & DT_PerfectBlock) ~= 0 then
		damageTypeStr = "PerfectBlock"
	elseif (swingData.damageType & DT_Block) ~= 0 then
		damageTypeStr = "Block"
	else
		damageTypeStr = ""
	end

	if outputFile then
		outputFile:write(string.format(
		        "%u\t%s\t%s\t%s\t%s\t%s\t%d\t%s\t%s\n",
		        swingData.time,
		        swingData.attacker,
		        swingData.victim,
		        swingData.attackName,
		        swingTypeStr,
		        damageTypeStr,
		        swingData.damage,
		        swingData.critical and "crit" or "",
		        swingData.damageElement))
		outputFile:flush()
	end

	local buffer
	buffer = string.format(
	        "%s->%s: %s(%s%s%s) = %d%s(%s)",
	        swingData.attacker,
	        swingData.victim,
	        swingData.attackName,
	        swingTypeStr,
	        damageTypeStr and ", " or "",
	        damageTypeStr or "",
	        swingData.damage,
	        swingData.critical and "!" or "",
	        swingData.damageElement);

	print("Damage: " .. buffer)
end

local function onLoginResultMessage(packet)
	playerNameByHandle[packet.handle] = packet.name
end

local function onEnterMessage(packet)
	if packet.objType == EOT_Player then
		playerNameByHandle[packet.handle] = packet.playerInfo.szName
	elseif packet.objType == EOT_Summon then
		local name = packet.summonInfo.szName
		local masterHandle = packet.summonInfo.master_handle
		
		if playerNameByHandle[masterHandle] ~= nil then
			local masterName = playerNameByHandle[masterHandle]
			name = name .. " (Pet of " .. masterName .. ")"
		end
		
		playerNameByHandle[packet.handle] = name
	elseif packet.objType == EOT_Monster then
		playerNameByHandle[packet.handle] = "#mob|" .. packet.monsterInfo.monster_id
	end
end

local function onAttackEventMessage(packet)
	local swingData = {}
	
	showPacketJson(packet)
	
	swingData.attacker = getHandleName(packet.attacker_handle)
	swingData.victim = getHandleName(packet.target_handle)
	swingData.attackName = "Attack"
	swingData.time = utils.getTimeInMsec()
	
	for _, attackInfo in ipairs(packet.attack) do
		local hitType = 0
		
		if (attackInfo.flag & AIF_PerfectBlock) ~= 0 then
			hitType = hitType | DT_PerfectBlock
		end
		
		if (attackInfo.flag & AIF_Block) ~= 0 then
			hitType = hitType | DT_Block
		end
		
		if (attackInfo.flag & AIF_Miss) ~= 0 then
			hitType = hitType | DT_Miss
		end
		
		swingData.damageType = hitType
		
		if (attackInfo.flag & AIF_Critical) ~= 0 then
			swingData.critical = true
		else
			swingData.critical = false
		end
		
		
		if attackInfo.damage ~= 0 or swingData.damageType ~= 0 then
			if attackInfo.damage >= 0 then
				swingData.swingType = SwingType_Melee
			else
				swingData.swingType = SwingType_Healing
			end

			swingData.damage = attackInfo.damage
			swingData.damageElement = "Physical"
			sendCombatLogLine(swingData)
		end

		if attackInfo.mp_damage ~= 0 then
			swingData.swingType = SwingType_ManaDrain;
			swingData.damage = attackInfo.mp_damage
			swingData.damageElement = "Physical"
			sendCombatLogLine(swingData)
		end
	end
end

local ELEMENTAL_TYPE = {"None", "Fire", "Water", "Wind", "Earth", "Light", "Dark"};
local function onSkillMessage(packet)
	local swingData = {}

	showPacketJson(packet)

	if packet.type ~= ST_Fire and packet.type ~= ST_RegionFire then
		return
	end

	swingData.attacker = getHandleName(packet.caster)
	swingData.attackName = "#skill|" .. packet.skill_id .. "|" .. packet.skill_level
	swingData.time = utils.getTimeInMsec()

	for _, hit in ipairs(packet.fire.hits) do
		swingData.victim = getHandleName(hit.hTarget);

		if hit.type == SHT_DAMAGE or
		   hit.type == SHT_MAGIC_DAMAGE or
		   hit.type == SHT_DAMAGE_WITH_KNOCK_BACK or
		   hit.type == SHT_CHAIN_DAMAGE or
		   hit.type == SHT_CHAIN_MAGIC_DAMAGE
		then
			local damageInfo
			if hit.type == SHT_DAMAGE or hit.type == SHT_MAGIC_DAMAGE then
				damageInfo = hit.hitDamage.damage
			elseif hit.type == SHT_DAMAGE_WITH_KNOCK_BACK then
				damageInfo = hit.hitDamageWithKnockBack.damage
			else
				damageInfo = hit.hitChainDamage.damage
			end

			swingData.damageType = 0;

			if (damageInfo.flag & 1) ~= 0 then
				swingData.critical = true
			else
				swingData.critical = false
			end

			swingData.swingType = SwingType_NonMelee;

			if damageInfo.damage then
				swingData.damage = damageInfo.damage;
				swingData.damageElement = ELEMENTAL_TYPE[damageInfo.damage_type]
				sendCombatLogLine(swingData)
			end
		elseif hit.type == SHT_ADD_HP or hit.type == SHT_ADD_MP then
			local hitAddStat = hit.hitAddStat

			swingData.damageType = 0
			swingData.critical = false

			if hit.type == SHT_ADD_HP then
				if hitAddStat.nIncStat >= 0 then
					swingData.swingType = SwingType_Healing
				else
					swingData.swingType = SwingType_NonMelee
				end
			else
				if hitAddStat.nIncStat >= 0 then
					swingData.swingType = SwingType_ManaHealing
				else
					swingData.swingType = SwingType_ManaDrain
				end
			end

			swingData.damage = hitAddStat.nIncStat
			swingData.damageElement = "None"
			sendCombatLogLine(swingData)
		elseif hit.type == SHT_ADD_HP_MP_SP then
			local hitAddHPMPSP = hit.hitAddHPMPSP;

			swingData.damageType = 0
			swingData.critical = false

			swingData.damageElement = "None"

			if hitAddHPMPSP.nIncHP then
				swingData.damage = hitAddHPMPSP.nIncHP

				if hitAddHPMPSP.nIncHP >= 0 then
					swingData.swingType = SwingType_Healing
				else
					swingData.swingType = SwingType_NonMelee
				end

				sendCombatLogLine(swingData)
			end
			if hitAddHPMPSP.nIncMP then
				swingData.damage = hitAddHPMPSP.nIncMP

				if hitAddHPMPSP.nIncMP >= 0 then
					swingData.swingType = SwingType_ManaHealing
				else
					swingData.swingType = SwingType_ManaDrain
				end

				sendCombatLogLine(swingData)
			end
		elseif hit.type == SHT_CHAIN_HEAL then
			local hitChainHeal = hit.hitChainHeal

			swingData.damageType = 0
			swingData.critical = false

			swingData.damageElement = "None"

			swingData.damage = hitChainHeal.nIncHP

			if hitChainHeal.nIncHP >= 0 then
				swingData.swingType = SwingType_Healing
			else
				swingData.swingType = SwingType_NonMelee
			end

			sendCombatLogLine(swingData)
		elseif hit.type == SHT_REBIRTH then
			local hitRebirth = hit.hitRebirth

			swingData.damageType = 0
			swingData.critical = false

			swingData.damageElement = "None"

			if hitRebirth.nIncHP then
				swingData.damage = hitRebirth.nIncHP
				swingData.swingType = SwingType_Healing
				sendCombatLogLine(swingData)
			end
			if hitRebirth.nIncMP then
				swingData.damage = hitRebirth.nIncMP
				swingData.swingType = SwingType_ManaHealing
				sendCombatLogLine(swingData)
			end
		elseif hit.type == SHT_RESULT then
			local hitResult = hit.hitResult

			if hitResult.bResult == false then
				swingData.damageType = DT_Miss
				swingData.critical = false

				swingData.damageElement = "None"

				swingData.damage = 0
				swingData.swingType = SwingType_NonMelee
				sendCombatLogLine(swingData)
			end
		end
	end
end

local function onStateResultMessage(packet)
	local swingData = {}

	showPacketJson(packet)

	swingData.attacker = getHandleName(packet.caster_handle)
	swingData.victim = getHandleName(packet.target_handle)
	swingData.attackName = "#dot|" .. packet.code .. "|" .. packet.level
	swingData.time = utils.getTimeInMsec()

	swingData.damageType = 0
	swingData.critical = false
	swingData.damage = packet.value
	swingData.damageElement = "None"

	if packet.result_type == SRT_Damage then
		if swingData.damage >= 0 then
			swingData.swingType = SwingType_NonMelee
		else
			swingData.swingType = SwingType_Healing
		end
	elseif packet.result_type == SRT_MagicDamage then
		if swingData.damage >= 0 then
			swingData.swingType = SwingType_ManaDrain
		else
			swingData.swingType = SwingType_ManaHealing
		end
	elseif packet.result_type == SRT_Heal then
		if swingData.damage >= 0 then
			swingData.swingType = SwingType_Healing
		else
			swingData.swingType = SwingType_NonMelee
		end
	elseif packet.result_type == SRT_MagicHeal then
		if swingData.damage >= 0 then
			swingData.swingType = SwingType_ManaHealing
		else
			swingData.swingType = SwingType_ManaDrain
		end
	else
		return
	end

	sendCombatLogLine(swingData)
end

local function onServerPacket(client, server, packet, serverType)
	if serverType ~= ST_Game then
		return true
	end
	
	if packet.id == TS_SC_LOGIN_RESULT then
		onLoginResultMessage(packet)
	elseif packet.id == TS_SC_ENTER then
		onEnterMessage(packet)
	elseif packet.id == TS_SC_ATTACK_EVENT then
		onAttackEventMessage(packet)
	elseif packet.id == TS_SC_SKILL then
		onSkillMessage(packet)
	elseif packet.id == TS_SC_STATE_RESULT then
		onStateResultMessage(packet)
	end
	
	return true
end

return {
    onServerPacket = onServerPacket
}