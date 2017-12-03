local function sendMessage(client, msg)
	packet = {}
	packet.id = TS_SC_CHAT
	packet.message = msg
	packet.szSender = "Filter"
	packet.type = 3 -- whisper
	
	client:sendPacket(packet)
end

return {
    sendMessage = sendMessage
}