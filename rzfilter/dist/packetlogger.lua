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