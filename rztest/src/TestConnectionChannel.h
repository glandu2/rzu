#ifndef TESTCONNECTIONCHANNEL_H
#define TESTCONNECTIONCHANNEL_H

#include <string>
#include <list>

#include "Extern.h"
#include "PacketSession.h"

#include <functional>
#include "gtest.h"

class RzTest;
class TestPacketServer;

#define AGET_PACKET(type_) \
	event.getPacket<type_>(); \
	ASSERT_EQ(TestConnectionChannel::Event::Packet, event.type); \
	ASSERT_NE(nullptr, packet)

class RZTEST_EXTERN TestConnectionChannel : public Object
{
	DECLARE_CLASS(TestConnectionChannel)
public:
	struct Event {
		enum Type {
			Connection,
			Disconnection,
			Packet
		};
		Type type;
		const TS_MESSAGE* packet;

		template<typename T>
		const T* getPacket() {
			const T* p = static_cast<const T*>(packet);
			if(!p)
				return nullptr;

			// force taking the value instead of reference to packetID
			EXPECT_EQ((const uint16_t)T::packetID, p->id);
			if(p->id == T::packetID)
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
	typedef std::function<void(TestConnectionChannel* channel)> YieldCallback;

public:
	TestConnectionChannel(Type type, cval<std::string>& host, cval<int>& port, bool encrypted)
		: type(type), host(host), port(port), encrypted(encrypted), session(nullptr), server(nullptr), test(nullptr) {}
	~TestConnectionChannel();

	void setTest(RzTest* test) { this->test = test; }

	void addCallback(EventCallback callback) { eventCallbacks.push_back(callback); }
	void addYield(YieldCallback callback, int time = 1);

	size_t getPendingCallbacks() { return eventCallbacks.size(); }
	int getPort();

	void start();
	void startClient();
	void startServer();

	void onEventReceived(PacketSession* session, Event::Type type);
	void onPacketReceived(PacketSession* session, const TS_MESSAGE* packet);

	void sendPacket(const TS_MESSAGE *packet);
	void closeSession();

	void registerSession(PacketSession* session);
	void unregisterSession();

protected:
	void callEventCallback(Event event, PacketSession* session);
	static void executeTimerForYield(uv_timer_t* timer);
	static void closeTimerForYield(uv_handle_t *timer);
	static TS_MESSAGE *copyMessage(const TS_MESSAGE *packet);

private:
	struct YieldData {
		uv_timer_t timer;
		YieldCallback callback;
		TestConnectionChannel *instance;
	};

private:
	Type type;
	cval<std::string>& host;
	cval<int>& port;
	bool encrypted;
	PacketSession* session;
	TestPacketServer* server;
	std::list<EventCallback> eventCallbacks;
	RzTest* test;
};

#endif // TESTCONNECTIONCHANNEL_H
