local forwardDisconnect = true

function onServerPacket(client, server, packet, serverType)
	--print("Received Server packet id " .. packet.id .. " (" .. packet.__name .. ")")	
	return not client:sendPacket(packet)
end

function onClientPacket(client, server, packet, serverType)
	--print("Received Client packet id " .. packet.id .. " (" .. packet.__name .. ")")

	if serverType == ST_Auth and packet.id == TS_CA_DISTRIBUTION_INFO then
		if packet.distributionInfo == "NODC" then
			forwardDisconnect = false
			print("Disabled disconnection forwarding")
		end
	elseif serverType == ST_Game and packet.id == TS_CS_CHAT_REQUEST then
		if packet.message == "NODC" then
			forwardDisconnect = false
			print("Disabled disconnection forwarding")
		end
	end
	
	return not server:sendPacket(packet)
end

function onUnknownPacket(fromEndpoint, toEndpoint, id, serverType, isServerPacket)
	print("Unknown packet " .. id)
	return true
end

function onClientDisconnected(client, server)
	print("Client disconnected")
	if forwardDisconnect == true then
		server:close()
	end
end

function onServerDisconnected(client, server)
	print("Server disconnected")
	if forwardDisconnect == true then
		client:close()
	end
end

