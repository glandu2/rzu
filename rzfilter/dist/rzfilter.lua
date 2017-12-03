local combatlog = require("combatlog")
local packetlogger = require("packetlogger")

local filterImplementation = combatlog

function onServerPacket(client, server, packet, serverType)
	--print("Received Server packet id " .. packet.id .. " (" .. packet.__name .. ")")
	--print(json.encode(packet))
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
end


function onClientPacket(client, server, packet, serverType)
	if filterImplementation and
	   filterImplementation.onClientPacket and
	   filterImplementation.onClientPacket(client, server, packet, serverType) ~= false
	then
		return not server:sendPacket(packet)
	end
--[[
	--print("Received Client packet id " .. packet.id)
	--print(json.encode(packet))
	if packet.id == TS_CS_GAME_GUARD_AUTH_ANSWER then
		return false
	end
	
	server:sendPacket(packet)
	return false
	--]]
	return true
end

