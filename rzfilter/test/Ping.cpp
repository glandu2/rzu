#include "gtest/gtest.h"
#include "Packets/TS_CA_ACCOUNT.h"
#include "Packets/TS_CA_SERVER_LIST.h"
#include "GlobalConfig.h"

#include "RC4Cipher.h"

#ifndef _WIN32
#define closesocket close
#endif

#pragma pack(push, 1)
struct TS_BIG : public TS_MESSAGE
{
	char account[1024];
	static const uint16_t packetID = 10010;
};
#pragma pack(pop)

class Ping_Test : public ::testing::Test {
public:
	virtual void SetUp() {
		outputSocket = outputServerSocket = inputSocket = -1;
		errno = 0;
		inputCipher.prepare("}h79q~B%al;k'y $E");
	}

	virtual void TearDown() {
		perror("last errno");

		if(inputSocket)
			closesocket(inputSocket);

		recv(outputSocket, recvBuffer, sizeof(recvBuffer), 0);
		if(outputSocket)
			closesocket(outputSocket);
		if(outputServerSocket)
			closesocket(outputServerSocket);
	}

	void doTest(TS_MESSAGE* testPacket) {
#ifdef _WIN32
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
		const int PING_LOOPS = CONFIG_GET()->count;

		struct sockaddr_in sin;
		struct sockaddr_in csin;
		uint64_t startTime, endTime;
		const int packetSize = testPacket->size;

		// Setup socket server
		outputServerSocket = socket(AF_INET, SOCK_STREAM, 0);
		ASSERT_NE(-1, outputServerSocket);

		memset(&sin, 0, sizeof(sin));
		sin.sin_addr.s_addr = inet_addr(CONFIG_GET()->output.ip.get().c_str());
		sin.sin_port = htons(CONFIG_GET()->output.port.get());
		sin.sin_family = AF_INET;

		ASSERT_NE(-1, bind(outputServerSocket, (struct sockaddr *) &sin, sizeof(sin)));
		ASSERT_NE(-1, listen(outputServerSocket, 5));

		// Setup client
		inputSocket = socket(AF_INET, SOCK_STREAM, 0);
		ASSERT_NE(-1, inputSocket);

		memset(&sin, 0, sizeof(sin));
		sin.sin_addr.s_addr = inet_addr(CONFIG_GET()->input.ip.get().c_str());
		sin.sin_port = htons(CONFIG_GET()->input.port.get());
		sin.sin_family = AF_INET;

		ASSERT_NE(-1, connect(inputSocket, (struct sockaddr *) &sin, sizeof(sin)));

		// Expect a connection on the server @ port 4800
		socklen_t csinSize = sizeof(csin);
		outputSocket = accept(outputServerSocket, (struct sockaddr *) &csin, &csinSize);
		ASSERT_NE(-1, outputSocket);

		int flag = 1;
		ASSERT_NE(-1, setsockopt(inputSocket, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(flag)));

		char** buffers = new char*[PING_LOOPS];
		for(int i = 0; i < PING_LOOPS; i++) {
			char* buffer = new char[packetSize];
			inputCipher.encode((const char*)testPacket, buffer, packetSize);
			buffers[i] = buffer;
		}

		startTime = uv_hrtime();
		for(int i = 0; i < PING_LOOPS; i++) {
			send(inputSocket, buffers[i], packetSize, 0);
			recv(outputSocket, recvBuffer, sizeof(recvBuffer), 0);
		}
		endTime = uv_hrtime();

		printf("Ping duration: %f ns\n", (endTime - startTime)/(float)PING_LOOPS);

		for(int i = 0; i < PING_LOOPS; i++) {
			char* buffer = buffers[i];
			delete[] buffer;
		}
		delete[] buffers;

#ifdef _WIN32
		WSACleanup();
#endif
	}

#ifdef _WIN32
	SOCKET outputSocket, outputServerSocket;
	SOCKET inputSocket;
#else
	int outputSocket, outputServerSocket;
	int inputSocket;
#endif
	char recvBuffer[4096];
	RC4Cipher inputCipher;
};

TEST_F(Ping_Test, latency_0_bytes_paquet) {
	TS_CA_SERVER_LIST packet;
	TS_MESSAGE::initMessage(&packet);

	doTest(&packet);
}

TEST_F(Ping_Test, latency_122_bytes_paquet) {
	TS_CA_ACCOUNT packet;
	TS_MESSAGE::initMessage(&packet);

	doTest(&packet);
}

TEST_F(Ping_Test, latency_1024_bytes_paquet) {
	TS_BIG packet;
	TS_MESSAGE::initMessage(&packet);

	doTest(&packet);
}
