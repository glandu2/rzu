#ifndef TESTCONNECTIONCHANNEL_H
#define TESTCONNECTIONCHANNEL_H

#include <string>
#include <list>

#include "Extern.h"
#include "PacketSession.h"

#include <functional>
#include "gtest.h"

class RzTest;

#define AGET_PACKET(type_) \
	event.getPacket<type_>(); \
	ASSERT_EQ(TestConnectionChannel::Event::Packet, event.type); \
	ASSERT_NE(nullptr, packet)

class RZTEST_EXTERN TestConnectionChannel : public Object
{
public:
	struct Event {
		enum Type {
			Connection,
			Disconnection,
			Packet
		};
		Type type;
		std::string origineName;
		const TS_MESSAGE* packet;

		template<typename T>
		const T* getPacket() {
			const T* p = static_cast<const T*>(packet);
			if(!p)
				return nullptr;
			else if(p->id == T::packetID)
				return p;
			else
				return nullptr;
		}
	};

	enum Type {
		Client,
		Server
	};

	//typedef void (*EventCallback)(RzTest* test, Event event, PacketSession* session);
	typedef std::function<void(TestConnectionChannel* channel, Event event)> EventCallback;

public:
	TestConnectionChannel(Type type, std::string name, bool encrypted) : type(type), name(name), encrypted(encrypted), session(nullptr) {}
	~TestConnectionChannel();

	void addCallback(EventCallback callback) { eventCallbacks.push_back(callback); }

	void start();
	void startClient();
	void startServer();

	void onEventReceived(PacketSession* session, Event::Type type);
	void onPacketReceived(PacketSession* session, const TS_MESSAGE* packet);

	void sendPacket(const TS_MESSAGE *packet);
	void closeSession();

	void registerSession(PacketSession* session);
	void unregisterSession();

	std::string getName() { return name; }

protected:
	void callEventCallback(std::string name, Event event, PacketSession* session);
	static std::string getHostname(std::string serverName);
	static TS_MESSAGE *copyMessage(const TS_MESSAGE *packet);

private:
	Type type;
	std::string name;
	bool encrypted;
	PacketSession* session;
	std::list<EventCallback> eventCallbacks;
};

#endif // TESTCONNECTIONCHANNEL_H
