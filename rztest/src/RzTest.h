#ifndef RZTEST_H
#define RZTEST_H

#include "Core/Object.h"
#include "Extern.h"
#include "TestConnectionChannel.h"
#include <list>

class TestPacketServer;
struct TS_MESSAGE;

class RZTEST_EXTERN RzTest : public Object {
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

#endif  // RZTEST_H
