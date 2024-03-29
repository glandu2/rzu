#pragma once

#include "Core/IDelegate.h"
#include "Core/Object.h"
#include "StreamAddress.h"
#include "uv.h"
#include <stdint.h>

#include <string>
#include <vector>

class IListener;
class Log;

class RZU_EXTERN Stream : public Object {
public:
	enum StreamType { ST_Socket, ST_Pipe };

	enum State {
		UnconnectedState,  // Client & server

		ConnectingState,  // Client
		BindingState,     // Server

		ListeningState,  // Server
		ConnectedState,  // Client

		ClosingState  // Client & Server
	};

	struct RZU_EXTERN WriteRequest {
		uv_write_t writeReq;
		uv_buf_t buffer;

		static WriteRequest* create(size_t dataSize);
		static WriteRequest* createFromExisting(char* buffer, size_t dataSize);
		static void destroy(WriteRequest* req);

	private:
		char data[];

	private:
		WriteRequest();
		~WriteRequest();
		WriteRequest(const WriteRequest&);
		WriteRequest& operator=(const WriteRequest&);
	};

	static const char* STATES[];

	typedef void (*CallbackOnDataReady)(IListener* instance, Stream* socket);
	typedef void (*CallbackOnStateChanged)(
	    IListener* instance, Stream* socket, State oldState, State newState, bool causedByRemote);
	typedef void (*CallbackOnError)(IListener* instance, Stream* socket, int errnoValue);

public:
	virtual ~Stream();

	static StreamType parseConnectionUrl(const char* url, std::string* target);
	static Stream* getStream(StreamType type,
	                         Stream* existingStream = nullptr,
	                         bool* changed = nullptr,
	                         bool enablePacketLogger = true);

	bool bindBeforeConnect(const std::string& bindIp, uint16_t bindPort);
	bool connect(const std::string& hostName, uint16_t port);
	bool listen(const std::string& interfaceIp, uint16_t port);

	size_t getAvailableBytes() { return recvBuffer.size(); }
	size_t read(void* buffer, size_t size);
	size_t readAll(std::vector<char>* buffer);  // data in buffer will be destroyed
	size_t discard(size_t size);                // read but does not store read data
	size_t discardAll();                        // discard all data
	size_t write(WriteRequest* writeRequest);   // take writeRequest's ownership
	size_t write(const void* buffer, size_t size);
	bool accept(Stream** clientSocket);

	void close(bool causedByRemote = false);
	void abort(bool causedByRemote = false);

	virtual void setKeepAlive(int delay) {}
	virtual void setNoDelay(bool enable) {}

	State getState() { return currentState; }

	virtual StreamAddress getRemoteAddress() = 0;
	virtual StreamAddress getLocalAddress() = 0;

	void addDataListener(IListener* instance, CallbackOnDataReady listener);
	void addConnectionListener(IListener* instance, CallbackOnDataReady listener);
	void addEventListener(IListener* instance, CallbackOnStateChanged listener);
	void addErrorListener(IListener* instance, CallbackOnError listener);
	void removeListener(IListener* instance);

	void onStreamError(int errorValue);

	void setLogPackedEnable(bool enable) { this->logPackets = enable; }
	void setPacketLogger(Log* packetLogger) { this->packetLogger = packetLogger; }
	Log* getPacketLogger() { return this->packetLogger; }

	void resetPacketTransferedFlag() { packetTransferedSinceLastCheck = false; }
	bool isPacketTransferedSinceLastCheck() { return packetTransferedSinceLastCheck; }

	void packetLog(Object::Level level, const unsigned char* rawData, int size, const char* format, ...)
	    PRINTFCHECK(5, 6);

protected:
	Stream(uv_loop_t* uvLoop, uv_stream_t* handle, bool logPackets = true);
	void setState(State state, bool causedByRemote = true);
	uv_loop_t* getLoop() { return loop; }

	static void onConnected(uv_connect_t* req, int status);
	static void onNewConnection(uv_stream_t* req, int status);

	static void onAllocReceiveBuffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
	static void onReadCompleted(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
	static void onWriteCompleted(uv_write_t* req, int status);

	static void onShutdownDone(uv_shutdown_t* req, int status);
	static void onConnectionClosed(uv_handle_t* handle);

protected:
	virtual int connect_impl(uv_connect_t* connectRequest, const std::string& hostName, uint16_t port) = 0;
	virtual int bind_impl(const std::string& interfaceIp, uint16_t port) = 0;
	virtual Stream* createStream_impl() = 0;

	virtual void onStateChanged(State oldState, State newState) {}

protected:
	bool logPackets;  // set to false when logging is done in a derived class

private:
	uv_loop_t* loop;
	uv_stream_t* handle;

	IDelegate<Stream::CallbackOnDataReady> dataListeners;
	IDelegate<Stream::CallbackOnDataReady> incomingConnectionListeners;
	IDelegate<Stream::CallbackOnStateChanged> eventListeners;
	IDelegate<Stream::CallbackOnError> errorListeners;

	uv_connect_t connectRequest;
	uv_shutdown_t shutdownReq;
	Stream::State currentState;

	std::vector<char> recvBuffer;

	Log* packetLogger;

	bool packetTransferedSinceLastCheck;  // when checking idle pipes, if this flag is false, then the pipe is closed
	                                      // (idle)
	bool closeCausedByRemote;
};
