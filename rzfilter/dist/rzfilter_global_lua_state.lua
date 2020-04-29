-- rzfilter_lua_global_state_module also use rzfilter.lua file, so to use this one you need to rename it to rzfilter.lua

-- LUA code outside any function is executed when rzfilter is started
-- If you modify code inside the lua script while rzfilter is running, it
-- will reload the rzfilter.lua, but this will lose all stored data in memory.
local timer = require("timer")

local globalVariable = 0

print("LUA script loaded")

function onClientConnected(client, server)
	print("Client connected")
	
	-- You can set arbitrary variables inside client.data and server.data.
	-- These variables will be stored alongside the endpoints.
	-- Store data here when you need to store data associated with a client connection.
	client.data.client_packet_io = 0
	client.data.server_packet_io = 0
	client.data.timer = timer:new()
	
	function onTimerCallback(t)
		client.data.server_packet_io = client.data.server_packet_io + 10000
		print("Timer tick, counter at " .. client.data.server_packet_io)
	end
	client.data.timer:start(onTimerCallback, 1000, 5000)
	
	-- This will modify the global variable
	-- Its new value will be visible for all connections
	globalVariable = globalVariable + 1
	print("Client connections: " .. globalVariable)
end

function onServerPacket(client, server, packet, serverType)
	print("Received Server packet id " .. packet.id .. " (" .. packet.__name .. ")")
	
	-- Increment the per-connection counter
	client.data.server_packet_io = client.data.server_packet_io + 1	
	return not client:sendPacket(packet)
end

function onClientPacket(client, server, packet, serverType)
	print("Received Client packet id " .. packet.id .. " (" .. packet.__name .. ")")
	
	-- Increment the per-connection counter
	client.data.client_packet_io = client.data.client_packet_io + 1
	return not server:sendPacket(packet)
end

function onUnknownPacket(fromEndpoint, toEndpoint, id, serverType, isServerPacket)
	if isServerPacket then
		client.data.server_packet_io = client.data.server_packet_io + 1	
	else
		client.data.client_packet_io = client.data.client_packet_io + 1
	end

	print("Unknown packet " .. id)
	return true
end

function onClientDisconnected(client, server)
	print("Client disconnected, client packets: " .. client.data.client_packet_io .. " server packets: " .. client.data.server_packet_io)
	
	-- You can create timer even if they run while the connection is closed.
	-- The timer won't be killed if it is running.
	function onTimerCallback(t)
		print("Stopping timer")
		t:stop()
	end
	client.data.timer:start(onTimerCallback, 1000, 0)

	server:close()
end

function onServerDisconnected(client, server)
	print("Server disconnected")
	client:close()
end
