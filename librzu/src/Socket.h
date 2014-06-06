#ifndef SOCKET_H
#define SOCKET_H

#include "Object.h"
#include "uv.h"
#include "IListener.h"
#include "stdint.h"
#include "IDelegate.h"
#include "Log.h"

#include <unordered_map>
#include <string>
#include <vector>

struct SocketInternal;
class SocketPoll;

class RAPPELZLIB_EXTERN Socket : public Object
{
	DECLARE_CLASS(Socket)

public:
	enum State {
		UnconnectedState,	//Client & server

		ConnectingState,	//Client
		BindingState,			//Server

		ListeningState,			//Server
		ConnectedState,		//Client

		ClosingState		//Client & Server
	};

	static const char* STATES[];

	typedef void (*CallbackOnDataReady)(IListener* instance, Socket* socket);
	typedef void (*CallbackOnStateChanged)(IListener* instance, Socket* socket, State oldState, State newState);
	typedef void (*CallbackOnError)(IListener* instance, Socket* socket, int errnoValue);

public:
	Socket(uv_loop_t* uvLoop, Log* packetLogger = nullptr, bool logPackets = true);
	virtual ~Socket();

	bool connect(const std::string& hostName, uint16_t port);
	bool listen(const std::string& interfaceIp, uint16_t port);

	size_t getAvailableBytes() { return recvBuffer.size(); }
	size_t read(void *buffer, size_t size);
	size_t readAll(std::vector<char>* buffer); //data in buffer will be destroyed
	size_t discard(size_t size); //read but does not store read data
	size_t write(const void *buffer, size_t size);
	bool accept(Socket *clientSocket);

	void close();
	void abort();

	State getState() { return currentState; }
	struct sockaddr_in getPeerInfo();
	const std::string& getHost() { return host; }
	uint16_t getPort() { return port; }

	void addDataListener(IListener* instance, CallbackOnDataReady listener);
	void addConnectionListener(IListener* instance, CallbackOnDataReady listener);
	void addEventListener(IListener* instance, CallbackOnStateChanged listener);
	void addErrorListener(IListener* instance, CallbackOnError listener);
	void removeListener(IListener* instance);

	void notifyReadyError(int errorValue);

	void setPacketLogger(Log* packetLogger) { this->packetLogger = packetLogger; }


protected:
	void setState(State state);
	void setPeerInfo(const std::string& host, uint16_t port);

	void packetLog(Log::Level level, const char *rawData, int size, const char* format, ...);


	static void onConnected(uv_connect_t* req, int status);
	static void onNewConnection(uv_stream_t* req, int status);

	static void onAllocReceiveBuffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
	static void onReadCompleted(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
	static void onWriteCompleted(uv_write_t* req, int status);

	static void onShutdownDone(uv_shutdown_t* req, int status);
	static void onConnectionClosed(uv_handle_t* handle);

private:
	uv_loop_t* uvLoop;
	std::string host;
	uint16_t port;

	IDelegate<Socket::CallbackOnDataReady> dataListeners;
	IDelegate<Socket::CallbackOnDataReady> incomingConnectionListeners;
	IDelegate<Socket::CallbackOnStateChanged> eventListeners;
	IDelegate<Socket::CallbackOnError> errorListeners;

	uv_tcp_t socket;
	uv_connect_t connectRequest;
	uv_shutdown_t shutdownReq;
	Socket::State currentState;

	std::vector<char> recvBuffer;
	bool socketInitialized; //for accept to prevent multiple init in case of failure of uv_accept

	Log* packetLogger;
	bool logPackets; //set to false when logging is done in a derived class (ie: RappelzSocket)
};

#endif // SOCKET_H
