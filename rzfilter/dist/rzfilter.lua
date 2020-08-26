local combatlog = require("combatlog")
local chattimestamps = require("chattimestamps")
local packetlogger = require("packetlogger")
local utils = require("utils")
local timer = require("timer")

print("New client connection, current time: " .. utils:getTimeInMsec())

-- Choose an functionality to use:
-- local filterImplementation = combatlog
-- local filterImplementation = chattimestamps
-- local filterImplementation = packetlogger

-- Documentation

-- rzfilter will instantiate a Lua VM with this file (rzfilter.lua) on each new client connection.
-- Variables created here (outside any functions) will be kept for the entire client connection lifetime.
-- rzfilter will call a lua function when it receives a packet:
--  - onServerPacket if it receive a packet from the server
--  - onClientPacket if it receive a packet from the client

-- These functions take the same arguments in this order:
--  - client: represent the client connection
--  - server: represent the server connection
--  - packet: the received packet as a lua table
--  - serverType: the connection type: auth or GS

-- client and server arguments match the C++ underlying type IFilterEndpoint.
-- they are objects with these functions:
--  - getPacketVersion()
--     Returns the connection packet version.
--     The version is represented in hex like this:
--       0xEESSPP where EE is the epic number, SS is the sub-epic number and PP the intermediate versions
--       For example 0x080100 match the first version of epic 8.1 and 0x080101 match a secondary version of 8.1 with RSA auth.
--       For a list of implemented epics, see here: https://github.com/glandu2/librzu/blob/master/src/lib/Packet/PacketEpics.h
--  - sendPacket(packet)
--     Send a packet over this connection.
--      The packet is a lua table where its field match the packet type fields
--  - close()
--     Close this connection
--  - getIp()
--     Returns a string with the remote IP. For client, this is the client's ip, for server, this is the server IP.
--     Might return an IPv6 if the connection is made with IPv6
--  - banIp(ip)
--     Ban an ip (string)

-- packet is a lua table containing the packet data.
-- There are several special fields for the packet given by rzfilter when onServerPacket or onClientPacket is called:
--  - __name
--     this field contains a string with the packet name (like "TS_SC_INVENTORY")
--  - __id
--     this field is an integer with the packet ID received from the client/server without modification
--  - id
--     this field is an integer with the packet ID matching the packet type from the latest known packet version
--     It can be different from __id in case the connection is not EPIC_LATEST.
--     For example, in epic 4, TS_SC_VERSION is ID 50 while in epic 9.5, it is 51.
--     in that case, __id will equal 50 (the true ID used between the server and client) and id will be set to 51
--     id is set like this to be able to compare it with TS_* constants.
--     For example, to test if a packet is a TS_SC_VERSION, use `packet.id == TS_SC_VERSION` (don't use __id for this)
--  - *
--     Other fields are the one from the packet.
--     See packetlogger.lua to dump them all in a sort of json format.

-- When creating a packet from scratch to send it using server:sendPacket(packet) or client:sendPacket(packet),
-- you must set the id field using one of TS_*.
-- then set each fields for the packet type to send.
-- See here to have information about packet fields name: https://github.com/glandu2/librzu/tree/master/src/packets/GameClient
-- For example, to create a TS_SC_CHAT packet, do this:
--   local packet = {}
--   packet.id = TS_SC_CHAT
--   packet.szSender = "SenderName"
--   packet.type = 3 -- 3 match CHAT_WHISPER in TS_CHAT_TYPE enum
--   packet.message = "message text to send to client chat"
--   server:sendPacket(packet)

-- The serverType argument indicate if the packet is exchanged between the auth and client or the GS and client.
-- It can have one of these values:
--  ST_Auth: auth <> client
--  ST_Game: GS <> client

-- These 2 functions (onServerPacket and onClientPacket) must be declared with this name for rzfilter to call them.
-- They can return a boolean to indicate if the received packet must be forwarded or not.
--  Return true will make the received packet to be fowarded
--  Return false will do nothing.
--  Modifying the "packet" argument won't be taken into account when forwarding the packet.
--  if you want to modify a packet, you must:
--   change it, for example `packet.message = "blabla"`
--   send it to the server or client like: server:sendPacket(packet)
--   return false in onServerPacket or onClientPacket
-- Returning nothing is the same as returning true.

-- If one of these functions is not declared, rzfilter will act as if the function was empty and returning true (ie: forward the packet)

-- rzfilter implements custom lua libraries:
-- - utils
-- - timer

--- utils library
-- Usage: `local utils = require("utils")`
-- utils is a library containing these functions:
--  - utils::getTimeInMsec()
--     Returns the current time in milisecond since epoch (01/01/1970 00:00 UTC)

--- timer library
-- Usage `local timer = require("timer")`
-- timer is a library containing these functions:
--  - timer::new()
--     Return a new timer instance
-- A timer instance have these functions:
--  - start(callback, timeout_ms, repeat_ms)
--     Start the timer. It will call `callback` function after `timeout_ms` ms after this start call
--      end then every `repeat_ms` ms.
--      The callback will be called with the timer instance as the only argument and must return nothing.
--     Returns an integer: < 0 in case of error, else the start is successful.
--  - stop()
--     Stop the timer. If it was started, the callback won't be called again.
--     After having stopped the timer, it is possible to restart it with start() call
--  - again()
--     Stop the timer, and if it is repeating restart it using the repeat value as the timeout.
--     If the timer has never been started before it returns UV_EINVAL.
--  - setRepeat(repeat_ms)
--     Set the repeat interval value in milliseconds.
--  - getRepeat()
--     Return the current repeat interval value in milliseconds.

-- Standard lua libraries are also available (via require()):
-- - package
-- - coroutine
-- - table
-- - io
-- - os
-- - string
-- - math
-- - utf8
-- - debug


--- Called when rzfilter receive a packet from the server (auth or GS) to be forwarded to the client
function onServerPacket(client, server, packet, serverType)
	-- If a filterImplementation is choosen run its onServerPacket function
	if filterImplementation and filterImplementation.onServerPacket
	then
		return filterImplementation.onServerPacket(client, server, packet, serverType)
	end
	
	-- Else return true to forward the packet as-is
	return true
end


--- Called when rzfilter receive a packet from the client to be forwarded to the server (auth or GS)
function onClientPacket(client, server, packet, serverType)
	-- If a filterImplementation is choosen run its onServerPacket function
	if filterImplementation and filterImplementation.onClientPacket
	then
		return filterImplementation.onClientPacket(client, server, packet, serverType)
	end

	-- Else return true to forward the packet as-is
	return true
end
