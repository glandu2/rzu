-- This filter add a timestamp to received chat message from the server
-- It sends the modified packets so the client display these timestamp

local function onServerPacket(client, server, packet, serverType)
	if serverType ~= ST_Game then
		return true
	end

	-- Filter only chat packets from the server
	if packet.id == TS_SC_CHAT then
		-- If it is a system message with a string reference (using @xxxx), send a message with the timestamp before the system message
		if string.sub(packet.szSender, 1, 1) == '@' then
			if packet.szSender == "@PARTY" or packet.szSender == "@GUILD" then
				return true
			end
			packet.message = "<b>" .. os.date("%X") .. ":</b> Next message: " .. packet.szSender
			packet.szSender = "Filter"
			packet.type = 3
			client:sendPacket(packet)
		else
			-- Else if it is a normal chat message (from an user for example), prefix the message with the timestamp
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
