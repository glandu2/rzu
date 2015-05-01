#include "gtest/gtest.h"
#include "RzTest.h"
#include "Packets/TS_CA_ACCOUNT.h"
#include "Packets/TS_CA_SERVER_LIST.h"
#include "GlobalConfig.h"

#include "RC4Cipher.h"

#ifndef _WIN32
#define closesocket close
#endif

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

TEST_F(Ping_Test, raw_latency) {
#ifdef _WIN32
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
	const int PING_LOOPS = CONFIG_GET()->count;

	struct sockaddr_in sin;
	struct sockaddr_in csin;
	uint64_t startTime, endTime;
	TS_CA_SERVER_LIST testPacket;
	TS_MESSAGE::initMessage(&testPacket);

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

	char** buffers = new char*[PING_LOOPS];
	for(int i = 0; i < PING_LOOPS; i++) {
		char* buffer = new char[sizeof(testPacket)];
		inputCipher.encode((const char*)&testPacket, buffer, sizeof(testPacket));
		buffers[i] = buffer;
	}

	startTime = uv_hrtime();
	for(int i = 0; i < PING_LOOPS; i++) {
		send(inputSocket, buffers[i], sizeof(testPacket), 0);
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
