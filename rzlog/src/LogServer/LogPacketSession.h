#ifndef LOGSERVER_LOGPACKETSESSION_H
#define LOGSERVER_LOGPACKETSESSION_H

#include "LS_11N4S.h"
#include "NetSession/SocketSession.h"
#include "Stream/Stream.h"

class SessionServerCommon;

namespace LogServer {

class LogPacketSession : public SocketSession {
	DECLARE_CLASS(LogServer::LogPacketSession)

public:
	static const uint16_t ALL_PACKETS = 0xFFFE;
	static const uint32_t MAX_PACKET_SIZE = 65536;

private:
	static const uint32_t INITIAL_INPUT_BUFFERSIZE = 296;
	struct InputBuffer {
		uint8_t* buffer;
		uint32_t bufferSize;
		struct MiniHeader {
			uint16_t id;
			uint16_t size;
		} currentMessage;
		bool discardPacket;
	};

public:
	LogPacketSession();
	virtual ~LogPacketSession();

	void sendPacket(const LS_11N4S* data);

	virtual bool hasCustomPacketLogger() { return true; }
	static bool hasCustomPacketLoggerStatic() { return true; }

protected:
	virtual EventChain<LogPacketSession> onPacketReceived(const LS_11N4S* packet) {
		return EventChain<LogPacketSession>();
	}

	void dispatchPacket(const LS_11N4S* packetData);
	virtual void logPacket(bool outgoing, const LS_11N4S* msg);

private:
	EventChain<SocketSession> onDataReceived();

private:
	InputBuffer inputBuffer;
};

}  // namespace LogServer

#endif  // PACKETSESSION_H
