#include "BenchmarkLogSession.h"

#include <string.h>

#include "LS_11N4S.h"

BenchmarkLogSession::BenchmarkLogSession(BenchmarkConfig* config) {
	this->config = config;

	LS_11N4S* packet = reinterpret_cast<LS_11N4S*>(buffer);
	packet->thread_id = 3628;
	packet->type = 1;
	packet->id = 101;
	packet->n1 = 3;
	packet->n2 = 9;
	packet->n3 = 0;
	packet->n4 = 3014;
	packet->n5 = 0;
	packet->n6 = 0;
	packet->n7 = 0;
	packet->n8 = 0;
	packet->n9 = 168059;
	packet->n10 = 51421;
	packet->n11 = 0;

	packet->len1 = 5;
	packet->len2 = 6;
	packet->len3 = 0;
	packet->len4 = 7;

	packet->size = sizeof(*packet) + packet->len1 + packet->len2 + packet->len3 + packet->len4;

	char* p = buffer + sizeof(*packet);
	strcpy(p, "test1Testungame001");
}

EventChain<SocketSession> BenchmarkLogSession::onConnected() {
	sendPackets();
	delayTimer.start(this, &BenchmarkLogSession::sendPackets, config->delay, config->delay);
	return SocketSession::onConnected();
}

void BenchmarkLogSession::sendPackets() {
	for(int i = 0; i < config->packetPerSalve && config->packetSent < config->packetTargetCount; i++) {
		config->packetSent++;
		LS_11N4S* packet = reinterpret_cast<LS_11N4S*>(buffer);
		write(buffer, packet->size);
		packet->id = 6546;
	}

	if(config->packetSent >= config->packetTargetCount) {
		closeSession();
	}
}
