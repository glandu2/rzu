#pragma once

#include "Core/IDelegate.h"
#include "PacketSession.h"

class RZU_EXTERN DelegatedPacketSession : public PacketSession {
	DECLARE_CLASS(DelegatedPacketSession)
public:
	typedef void (*CallbackFunction)(IListener* instance, PacketSession* server, const TS_MESSAGE* packetData);

public:
	void addPacketListener(uint16_t packetId, IListener* instance, CallbackFunction onPacketReceivedCallback);
	void removePacketListener(uint16_t packetId, IListener* instance);

protected:
	EventChain<PacketSession> onPacketReceived(const TS_MESSAGE* packet);

private:
	IDelegateHash<uint16_t, CallbackFunction> packetListeners;
};

