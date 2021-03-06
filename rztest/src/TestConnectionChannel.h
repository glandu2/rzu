#pragma once

#include <list>
#include <string>

#include "Core/Object.h"
#include "Core/Timer.h"
#include "Extern.h"
#include "Packet/PacketBaseMessage.h"

#include "gtest/gtest.h"
#include <functional>

class RzTest;
class TestPacketServer;
class PacketSession;
template<typename T> class cval;

#define AGET_PACKET(type_) \
	event.getPacket<type_>(); \
	ASSERT_EQ(TestConnectionChannel::Event::Packet, event.type); \
	ASSERT_NE(nullptr, packet)

#define AGET_PACKET_WITH_ID(type_, id_) \
	event.getPacket<type_>(id_); \
	ASSERT_EQ(TestConnectionChannel::Event::Packet, event.type); \
	ASSERT_NE(nullptr, packet)

#define DESERIALIZE_PACKET(target, version) \
	ASSERT_EQ(TestConnectionChannel::Event::Packet, event.type); \
	ASSERT_EQ(true, event.deserializePacket(target, version));

/**
 * @brief Test network operation with a remote server or client
 * This class allow to execute callbacks on event reception (received packet of connection/disconnection events)
 * These callbacks are meant to test if the network event is expected and send network packets
 */
class RZTEST_EXTERN TestConnectionChannel : public Object {
	DECLARE_CLASS(TestConnectionChannel)
public:
	struct Event {
		enum Type { Connection, Disconnection, Packet };
		Type type;
		const TS_MESSAGE* packet;

		template<typename T>
		typename std::enable_if<std::is_base_of<TS_MESSAGE, T>::value, const T*>::type getPacket(
		    uint16_t expectedId = T::packetID) {
			const T* p = static_cast<const T*>(packet);
			if(!p)
				return nullptr;

			// force taking the value instead of reference to packetID
			EXPECT_EQ(expectedId, p->id);
			if(p->id == expectedId)
				return p;
			else
				return nullptr;
		}

		template<typename T> bool deserializePacket(T& p, int version) {
			// force taking the value instead of reference to packetID
			EXPECT_EQ(p.getId(version), packet->id);
			if(packet->id != p.getId(version))
				return false;

			return packet->process(p, version);
		}
	};

	enum Type { Client, Server };

	// typedef void (*EventCallback)(RzTest* test, Event event, PacketSession* session);
	typedef std::function<void(TestConnectionChannel* channel, Event event)> EventCallback;
	typedef std::function<void(TestConnectionChannel* channel)> YieldCallback;

	template<class T>
	typename std::enable_if<!std::is_pointer<T>::value, void>::type sendPacket(const T& data,
	                                                                           packet_version_t version) {
		MessageBuffer buffer(data.getSize(version), version);
		data.serialize(&buffer);
		sendPacket((const TS_MESSAGE*) buffer.getData());
	}

public:
	TestConnectionChannel(Type type, cval<std::string>& host, cval<int>& port, bool encrypted);
	TestConnectionChannel(const TestConnectionChannel&) = delete;
	~TestConnectionChannel();

	void setTest(RzTest* test) { this->test = test; }

	/**
	 * @brief addCallback
	 * Add an event callback step
	 * Callbacks are called in added order at each received network event
	 * They allow to test if the event is expected and send network packets (to continue the network communication)
	 * @param callback the callback to call on event (each callback is called one time in order they are added)
	 */
	void addCallback(EventCallback callback) { eventCallbacks.push_back(callback); }

	/**
	 * @brief addYield
	 * Add a delayed callback
	 * The callback will be called once after the specified time
	 * @param callback the callback to call
	 * @param time delay time in ms
	 */
	void addYield(YieldCallback callback, int time = 1);

	/**
	 * @brief getPendingCallbacks
	 * Get the number of callback not triggered
	 * The number can be > 0 after the test if there are events that were expected (and so a callback was registered for
	 * them) but didn't happen This function is used by RzTest to check that all callbacks were triggered after each
	 * test
	 * @return the number of pending callbacks
	 */
	size_t getPendingCallbacks() { return eventCallbacks.size(); }

	/**
	 * @brief getPort
	 * Used by RzTest when callbacks are still pending after a test,
	 * to print the port associated with the channel (to identify in logs which channel still have callbacks pending
	 * after a test)
	 * @return associated port
	 */
	int getPort();

	/**
	 * @brief start the channel
	 * start a client connection or a server listening for a connection (based on constructor parameter type)
	 */
	void start();

	/**
	 * @brief onEventReceived
	 * Called by TestPacketSession when an event (connection or disconnection) happen
	 * @param session the client session (local or remote)
	 * @param type the event type (connection or disconnection)
	 */
	void onEventReceived(PacketSession* session, Event::Type type);

	/**
	 * @brief onPacketReceived
	 * Called by TestPacketSession when a packet is received
	 * @param session the client session (local or remote)
	 * @param packet the received packet
	 */
	void onPacketReceived(PacketSession* session, const TS_MESSAGE* packet);

	/**
	 * @brief sendPacket
	 * Send a packet to remote end
	 * @param packet the packet to send
	 */
	void sendPacket(const TS_MESSAGE* packet);

	/**
	 * @brief closeSession
	 * close the channel (and stop the server if applicable)
	 */
	void closeSession();

	/**
	 * @brief abortTest
	 * remove all callbacks and close the channel
	 */
	void abortTest();

	/**
	 * @brief registerSession
	 * Ensure only one client is up at all time (espacially when the test is listening a connection)
	 * Events received by the session are forwarded to TestConnectionChannel
	 * @param session the client session (local or remote) being associated with the test channel
	 */
	void registerSession(PacketSession* session);

	/**
	 * @brief unregisterSession
	 * unbind a client session with the channel (when disconnected)
	 * @param session the session to unbind
	 */
	void unregisterSession(PacketSession* session);

	bool isConnected();

protected:
	void callEventCallback(Event event, PacketSession* session);
	static TS_MESSAGE* copyMessage(const TS_MESSAGE* packet);

	void startClient();
	void startServer();

private:
	struct YieldData {
		Timer<YieldData> timer;
		YieldCallback callback;
		TestConnectionChannel* instance;

		void executeTimerForYield();
	};

private:
	Type type;
	cval<std::string>& host;
	cval<int>& port;
	PacketSession* session;
	TestPacketServer* server;
	std::list<EventCallback> eventCallbacks;
	RzTest* test;
};
