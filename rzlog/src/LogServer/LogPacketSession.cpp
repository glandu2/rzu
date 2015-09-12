#include "LogPacketSession.h"

namespace LogServer {

LogPacketSession::LogPacketSession() {
	inputBuffer.bufferSize = INITIAL_INPUT_BUFFERSIZE;
	inputBuffer.buffer = new uint8_t[inputBuffer.bufferSize];
	inputBuffer.currentMessage.id = 0;
	inputBuffer.currentMessage.size = 0;
	inputBuffer.discardPacket = false;

}

LogPacketSession::~LogPacketSession() {
	delete[] inputBuffer.buffer;
}

void LogPacketSession::sendPacket(const LS_11N4S* data) {
	write(data, data->size);

	//Log after for better latency
	logPacket(true, data);
}

void LogPacketSession::dispatchPacket(const LS_11N4S* packetData) {
	//Log before to avoid having logging a packet send after having received this packet before logging this packet
	logPacket(false, packetData);

	onPacketReceived(packetData);
}

void LogPacketSession::logPacket(bool outgoing, const LS_11N4S* msg) {
	log(LL_Trace, "Packet %s id: %5d, size: %d\n",
		  (outgoing)? "out" : " in",
		  msg->id,
		  int(msg->size));

	getStream()->packetLog(LL_Debug, reinterpret_cast<const unsigned char*>(msg), (int)msg->size,
			  "Packet %s id: %5d, size: %d\n",
			  (outgoing)? "out" : "in ",
			  msg->id,
			  int(msg->size));
}

void LogPacketSession::onDataReceived() {
	Stream* inputStream = getStream();
	InputBuffer* buffer = &inputBuffer;

	do {
		// if buffer->currentMessage.size == 0 => waiting for a new message
		if(buffer->currentMessage.size == 0 && inputStream->getAvailableBytes() < 4) {
			return;
		} else if(buffer->currentMessage.size == 0) {
			read(&buffer->currentMessage, 4);
			if(buffer->currentMessage.size <= 4)
				buffer->currentMessage.size = 0;
			buffer->discardPacket = buffer->currentMessage.size > MAX_PACKET_SIZE;

			buffer->currentMessage.size -= 4;
		}

		if(buffer->currentMessage.size != 0 && buffer->discardPacket) {
			buffer->currentMessage.size -= (uint32_t) inputStream->discard(buffer->currentMessage.size);
		} else if(buffer->currentMessage.size != 0 && inputStream->getAvailableBytes() >= buffer->currentMessage.size) {
			if(buffer->currentMessage.size+4u > buffer->bufferSize) {
				if(buffer->bufferSize)
					delete[] buffer->buffer;
				buffer->bufferSize = buffer->currentMessage.size+4;
				buffer->buffer = new uint8_t[buffer->currentMessage.size+4];
			}
			reinterpret_cast<LS_11N4S*>(buffer->buffer)->id = buffer->currentMessage.id;
			reinterpret_cast<LS_11N4S*>(buffer->buffer)->size = buffer->currentMessage.size+4;
			read(buffer->buffer + 4, buffer->currentMessage.size);
			dispatchPacket(reinterpret_cast<LS_11N4S*>(buffer->buffer));

			buffer->currentMessage.size = 0;
		}
	} while((buffer->currentMessage.size == 0 && inputStream->getAvailableBytes() >= 4) || (buffer->currentMessage.size != 0 && inputStream->getAvailableBytes() >= buffer->currentMessage.size));
}

} // namespace LogServer
