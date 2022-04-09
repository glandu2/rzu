#pragma once

#include "Packet/MessageBuffer.h"

struct TS_MESSAGE;
class ConnectionToServer;

class IPacketInterface {
public:
	virtual ~IPacketInterface() {}
	virtual void onPacketFromClient(const TS_MESSAGE* packet) = 0;
	virtual void onPacketFromServer(const TS_MESSAGE* packet) = 0;
	virtual packet_version_t getPacketVersion() = 0;

	template<class T>
	typename std::enable_if<!std::is_pointer<T>::value, void>::type onPacketFromClient(const T& data) {
		MessageBuffer buffer(data.getSize(getPacketVersion()), getPacketVersion());
		data.serialize(&buffer);
		if(buffer.checkPacketFinalSize() == false) {
		} else {
			onPacketFromClient((const TS_MESSAGE*) buffer.getData());
		}
	}

	template<class T>
	typename std::enable_if<!std::is_pointer<T>::value, void>::type onPacketFromServer(const T& data) {
		MessageBuffer buffer(data.getSize(getPacketVersion()), getPacketVersion());
		data.serialize(&buffer);
		if(buffer.checkPacketFinalSize() == false) {
		} else {
			onPacketFromServer((const TS_MESSAGE*) buffer.getData());
		}
	}
};
