#ifndef RZTEST_H
#define RZTEST_H

#include "Object.h"
#include <list>
#include "TestConnectionChannel.h"
#include "Extern.h"

class TestPacketServer;
struct TS_MESSAGE;

class RZTEST_EXTERN RzTest : public Object
{
	DECLARE_CLASS(RzTest)
public:

public:
	RzTest();

	void addChannel(TestConnectionChannel* channel);
	void abortTest();
	void run();

protected:

private:
	std::list<TestConnectionChannel*> channels;
	std::vector<char*> testedExecArgs;
};

#endif // RZTEST_H