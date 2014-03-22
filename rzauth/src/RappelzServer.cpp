#include "RappelzServer.h"
#include "EventLoop.h"
#include "SocketSession.h"
#include "BanManager.h"

RappelzServerCommon::RappelzServerCommon() : openServer(false), serverSocket(new Socket(EventLoop::getLoop())), lastWaitingInstance(nullptr), banManager(nullptr) {
	serverSocket->addConnectionListener(this, &onNewConnection);
}

RappelzServerCommon::~RappelzServerCommon() {
	stop();

	if(lastWaitingInstance)
		delete lastWaitingInstance;

	serverSocket->deleteLater();
}

void RappelzServerCommon::startServer(const std::string &interfaceIp, uint16_t port, BanManager *banManager) {
	this->banManager = banManager;
	openServer = true;
	serverSocket->listen(interfaceIp, port);
}

void RappelzServerCommon::stop() {
	serverSocket->close();
	openServer = false;
	for(auto it = sockets.begin(); it != sockets.end();) {
		(*it)->close();
		it = sockets.erase(it);
	}
}

void RappelzServerCommon::onNewConnection(IListener* instance, Socket* serverSocket) {
	RappelzServerCommon* thisInstance = static_cast<RappelzServerCommon*>(instance);

	if(!thisInstance->openServer)
		return;

	if(thisInstance->lastWaitingInstance == nullptr) {
		thisInstance->lastWaitingInstance = thisInstance->createSession();
		thisInstance->lastWaitingInstance->socket->addEventListener(thisInstance->lastWaitingInstance, &onSocketStateChanged);
	}

	if(serverSocket->accept(thisInstance->lastWaitingInstance->getSocket())) {
		if(thisInstance->banManager && thisInstance->banManager->isBanned(thisInstance->lastWaitingInstance->getSocket()->getPeerInfo().sin_addr.s_addr)) {
			thisInstance->lastWaitingInstance->getSocket()->abort();
			thisInstance->info("Kick banned ip %s\n", thisInstance->lastWaitingInstance->getSocket()->getHost().c_str());
		} else {
			thisInstance->sockets.push_back(thisInstance->lastWaitingInstance->getSocket());
			thisInstance->lastWaitingInstance->setServer(thisInstance, --thisInstance->sockets.end());
		}
		thisInstance->lastWaitingInstance = nullptr;
	}
}


void RappelzServerCommon::onSocketStateChanged(IListener* instance, Socket*, Socket::State, Socket::State newState) {
	SocketSession* thisInstance = static_cast<SocketSession*>(instance);

	if(newState == Socket::UnconnectedState) {
		delete thisInstance;
	}
}