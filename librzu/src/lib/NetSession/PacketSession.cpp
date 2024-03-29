#include "PacketSession.h"

#include "Config/GlobalCoreConfig.h"
#include "Core/Log.h"
#include "Packet/JSONWriter.h"
#include "Packet/PacketBaseMessage.h"
#include "Packet/PacketStructsName.h"
#include "PacketIterator.h"

PacketSession::PacketSession(SessionType sessionType, SessionPacketOrigin packetOrigin, int version)
    : packetVersion(version),
      sessionType(sessionType),
      packetOrigin(packetOrigin),
      dumpRawPackets(GlobalCoreConfig::get()->packetLog.dumpRaw),
      dumpJsonPackets(GlobalCoreConfig::get()->packetLog.dumpJson) {
	inputBuffer.bufferSize = INITIAL_INPUT_BUFFERSIZE;
	inputBuffer.buffer = new uint8_t[inputBuffer.bufferSize];
	inputBuffer.currentMessageSize = 0;
	inputBuffer.discardPacket = false;
}

PacketSession::~PacketSession() {
	delete[] inputBuffer.buffer;
}

void PacketSession::sendPacket(const TS_MESSAGE* data) {
	write(data, data->size);

	// Log after for better latency
	logPacket(true, data);
}

EventChain<SocketSession> PacketSession::onConnected() {
	inputBuffer.currentMessageSize = 0;
	inputBuffer.discardPacket = false;
	return SocketSession::onConnected();
}

void PacketSession::dispatchPacket(const TS_MESSAGE* packetData) {
	preProcessPacketReceived(packetData);

	// Log before to avoid having logging a packet send after having received this packet before logging this packet
	logPacket(false, packetData);

	onPacketReceived(packetData);
}

template<class Packet>
RZU_EXTERN void PacketSession::logPacketJson(
    const Packet* packet, bool isValid, packet_version_t version, bool outgoing, bool* ok) {
	if(!dumpJsonPackets.get())
		return;

	JSONWriter jsonWriter(version, false);
	packet->serialize(&jsonWriter);
	jsonWriter.finalize();
	std::string jsonData = jsonWriter.toString();

	Log* packetLogger = getStream()->getPacketLogger();
	Log::Level level = LL_Debug;

	if(!packetLogger) {
		packetLogger = Log::get();
		level = LL_Trace;
	}

	const char* fromName = nullptr;
	const char* toName = nullptr;

	PacketMetadata::getPacketOriginName(outgoing, sessionType, packetOrigin, &fromName, &toName);

	packetLogger->log(level,
	                  getStream(),
	                  "%sPacket json %s, %s -> %s id: %5d (%s):\n%s\n",
	                  isValid ? "" : "Partial invalid ",
	                  (outgoing) ? "out" : "in",
	                  fromName,
	                  toName,
	                  outgoing ? packet->getId(version) : packet->getReceivedId(),
	                  packet->getName(),
	                  jsonData.c_str());

	if(ok)
		*ok = isValid;
}

template<class Packet> struct PacketSession::PrintPacketFunctor {
	bool operator()(PacketSession* self, const TS_MESSAGE* packet, packet_version_t version, bool outgoing, bool* ok) {
		packet->processEvenIfInvalid(self, &PacketSession::logPacketJson<Packet>, version, version, outgoing, ok);
		return true;
	}
};

bool PacketSession::logPacketEnabled() {
	return getStream() && getStream()->getPacketLogger() && getStream()->getPacketLogger()->wouldLog(LL_Debug);
}

void PacketSession::logPacket(bool outgoing, const TS_MESSAGE* msg) {
	if(!logPacketEnabled() || msg->id == 9999)
		return;

	logPacketWithoutJson(outgoing, msg, nullptr);

	if(dumpJsonPackets.get()) {
		bool dummy;
		bool ok = false;
		SessionPacketOrigin origin = PacketMetadata::getPacketOriginFromDirection(outgoing, packetOrigin);

		processPacket<PrintPacketFunctor>(
		    sessionType, origin, packetVersion, msg->id, dummy, this, msg, packetVersion, outgoing, &ok);

		if(!ok) {
			const char* packetName = nullptr;
			const char* fromName = nullptr;
			const char* toName = nullptr;

			PacketMetadata::getPacketOriginName(outgoing, sessionType, packetOrigin, &fromName, &toName);

			// Guess packet name from ID
			packetName = PacketMetadata::getPacketName(msg->id, sessionType, origin, packetVersion);

			if(!packetName)
				packetName = "unknown packet";

			log(Object::LL_Debug, "Can't log json for packet id %d (%s)\n", msg->id, packetName);
			getStream()->packetLog(LL_Debug,
			                       reinterpret_cast<const unsigned char*>(msg) + sizeof(TS_MESSAGE),
			                       (int) msg->size - sizeof(TS_MESSAGE),
			                       "Packet %s, %s -> %s id: %5d (%s), size: %d\n",
			                       (outgoing) ? "out" : "in",
			                       fromName,
			                       toName,
			                       msg->id,
			                       packetName,
			                       int(msg->size - sizeof(TS_MESSAGE)));
		}
	}
}

void PacketSession::logPacketWithoutJson(bool outgoing, const TS_MESSAGE* msg, const char* packetName) {
	if(!dumpRawPackets.get())
		return;

	// If packetName is not known, guess it from ID
	if(!packetName)
		packetName = PacketMetadata::getPacketName(
		    msg->id, sessionType, PacketMetadata::getPacketOriginFromDirection(outgoing, packetOrigin), packetVersion);

	if(!packetName)
		packetName = "unknown";

	const char* fromName = nullptr;
	const char* toName = nullptr;

	PacketMetadata::getPacketOriginName(outgoing, sessionType, packetOrigin, &fromName, &toName);

	log(LL_Trace,
	    "Packet %s, %s -> %s id: %5d (%s), size: %d\n",
	    (outgoing) ? "out" : "in",
	    fromName,
	    toName,
	    msg->id,
	    packetName,
	    int(msg->size - sizeof(TS_MESSAGE)));

	getStream()->packetLog(LL_Debug,
	                       reinterpret_cast<const unsigned char*>(msg) + sizeof(TS_MESSAGE),
	                       (int) msg->size - sizeof(TS_MESSAGE),
	                       "Packet %s, %s -> %s id: %5d (%s), size: %d\n",
	                       (outgoing) ? "out" : "in",
	                       fromName,
	                       toName,
	                       msg->id,
	                       packetName,
	                       int(msg->size - sizeof(TS_MESSAGE)));
}

EventChain<SocketSession> PacketSession::onDataReceived() {
	Stream* inputStream = getStream();
	InputBuffer* buffer = &inputBuffer;

	do {
		// if buffer->currentMessageSize == 0 => waiting for a new message
		if(buffer->currentMessageSize == 0 && inputStream->getAvailableBytes() < 4) {
			break;
		} else if(buffer->currentMessageSize == 0) {
			read(&buffer->currentMessageSize, 4);
			if(buffer->currentMessageSize <= 4) {
				// Debug level to avoid flooding logs if receiving packets with 0s
				log(LL_Debug, "Invalid packet size of %u, tcp stream is probably desync\n", buffer->currentMessageSize);

				buffer->currentMessageSize = 0;
			} else {
				buffer->discardPacket = buffer->currentMessageSize > MAX_PACKET_SIZE;
				if(buffer->discardPacket) {
					log(LL_Warning,
					    "Packet size too big (%u > %u), will be discarded\n",
					    buffer->currentMessageSize,
					    MAX_PACKET_SIZE);
				}

				buffer->currentMessageSize -= 4;
			}
		}

		if(buffer->currentMessageSize != 0 && buffer->discardPacket) {
			buffer->currentMessageSize -= (uint32_t) inputStream->discard(buffer->currentMessageSize);
		} else if(buffer->currentMessageSize != 0 && inputStream->getAvailableBytes() >= buffer->currentMessageSize) {
			if(buffer->currentMessageSize + 4 > buffer->bufferSize) {
				if(buffer->bufferSize)
					delete[] buffer->buffer;
				buffer->bufferSize = buffer->currentMessageSize + 4;
				buffer->buffer = new uint8_t[buffer->currentMessageSize + 4];
			}
			reinterpret_cast<TS_MESSAGE*>(buffer->buffer)->size = buffer->currentMessageSize + 4;
			read(buffer->buffer + 4, buffer->currentMessageSize);
			dispatchPacket(reinterpret_cast<TS_MESSAGE*>(buffer->buffer));

			buffer->currentMessageSize = 0;
		}
	} while((buffer->currentMessageSize == 0 && inputStream->getAvailableBytes() >= 4) ||
	        (buffer->currentMessageSize != 0 && inputStream->getAvailableBytes() >= buffer->currentMessageSize));

	return SocketSession::onDataReceived();
}
