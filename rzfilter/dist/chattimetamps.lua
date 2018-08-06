-- This filter add a timestamp to received chat message from the server

local function onServerPacket(client, server, packet, serverType)
	if serverType ~= ST_Game then
		return true
	end

	if packet.id == TS_SC_CHAT then
		if string.sub(packet.szSender, 1, 1) == '@' then
			if packet.szSender == "@PARTY" or packet.szSender == "@GUILD" then
				return true
			end
			packet.message = "<b>" .. os.date("%X") .. ":</b> Next message: " .. packet.szSender
			packet.szSender = "Filter"
			packet.type = 3
			client:sendPacket(packet)
		else
			packet.message = "<b>" .. os.date("%X") .. ":</b>" .. packet.message
			client:sendPacket(packet)
			return false
		end
	end

	return true
end

return {
	onServerPacket = onServerPacket
}
