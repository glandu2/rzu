#ifndef ADMINSERVER_ADMININTERFACE_H
#define ADMINSERVER_ADMININTERFACE_H

namespace AdminServer {

//Manage the user interface
class AdminInterface
{
public:
	virtual void write(const void* data, int size) = 0;
	virtual void close() = 0;
};

} // namespace AdminServer

#endif // ADMINSERVER_ADMININTERFACE_H
