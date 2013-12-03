#include <QCoreApplication>
#include "PlayerCountMonitor.h"
#include <QStringList>
#include "Network/SocketPoll.h"
#include <thread>

SocketPoll socketPoll;

void updatePoll() {
	socketPoll.run();
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QByteArray host;
	quint16 port = 0;

	if(a.arguments().size() >= 3) {
		host = a.arguments().at(1).toUtf8();
		port = a.arguments().at(2).toUShort();
	}

	if(host.isNull() || port == 0) {
		host = "192.168.1.15"; //127.0.0.1";
		port = 4514;
	}

	PlayerCountMonitor playerCount(host.constData(), port);
	playerCount.start();

	new std::thread(&updatePoll);
	
	return a.exec();
}
