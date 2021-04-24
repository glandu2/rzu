#pragma once

#include "Core/Object.h"
#include "Core/Timer.h"
#include "Extern.h"
#include "TestConnectionChannel.h"
#include <list>

class TestPacketServer;
struct TS_MESSAGE;

class RZTEST_EXTERN RzTest : public Object {
	DECLARE_CLASS(RzTest)
public:
	RzTest();

	void addChannel(TestConnectionChannel* channel);
	void abortTest();
	void run(int timeoutMs = 0, std::function<void(void)> onAbortTest = std::function<void(void)>{});

protected:
	virtual void updateObjectName() override;
	void onTestTimeout();

private:
	std::list<TestConnectionChannel*> channels;
	std::vector<char*> testedExecArgs;
	Timer<RzTest> timeoutTimer;
	std::function<void(void)> onAbortTest;
};

