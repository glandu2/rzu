#include <QCoreApplication>
#include "PlayerCountMonitor.h"
#include <QStringList>

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
		host = "127.0.0.1";
		port = 4500;
	}

	PlayerCountMonitor playerCount(host.constData(), port);
	playerCount.start();
	
	return a.exec();
}
