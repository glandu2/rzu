-- rzfilter lua module that dump packets in JSON format from LUA.
-- This is not required to have JSON dumps now that the trafficdump supports dumping in JSON mode too
-- using trafficdump.dump_json.

local json = require("json")

function onServerPacket(client, server, packet, serverType)
	local jsonString = json.encode(packet)
	print("Received Server packet id " .. packet.__id .. " (" .. packet.__name .. ") " .. jsonString)
end


function onClientPacket(client, server, packet, serverType)
	local jsonString = json.encode(packet)
	print("Received Client packet id " .. packet.__id .. " (" .. packet.__name .. ") " .. jsonString)
end

return {
    onServerPacket = onServerPacket,
	onClientPacket = onClientPacket
}