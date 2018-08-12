local combatlog = require("combatlog")
local chattimestamps = require("chattimestamps")
local packetlogger = require("packetlogger")

local filterImplementation = combatlog
-- local filterImplementation = chattimestamps
-- local filterImplementation = packetlogger


function onServerPacket(client, server, packet, serverType)
	if filterImplementation and
	   filterImplementation.onServerPacket and
	   filterImplementation.onServerPacket(client, server, packet, serverType) ~= false
	then
		return not client:sendPacket(packet)
	end
	--[[
	if packet.id == TS_SC_GAME_GUARD_AUTH_QUERY then
		return false
	end
	
	client:sendPacket(packet)
	return false
	--]]
	return true
end


function onClientPacket(client, server, packet, serverType)
	if filterImplementation and
	   filterImplementation.onClientPacket and
	   filterImplementation.onClientPacket(client, server, packet, serverType) ~= false
	then
		return not server:sendPacket(packet)
	end

	--[[
	if packet.id == TS_CS_GAME_GUARD_AUTH_ANSWER then
		return false
	end
	
	server:sendPacket(packet)
	return false
	--]]
	return true
end

