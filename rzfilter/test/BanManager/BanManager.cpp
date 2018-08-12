#include "BanManagerEnvironment.h"
#include "GlobalConfig.h"
#include "RzTest.h"
#include "gtest/gtest.h"

TEST(BanManager, max_4_con_per_day__2_at_once) {
	RzTest test;
	TestConnectionChannel fakeServer(
	    TestConnectionChannel::Server, CONFIG_GET()->output.ip, CONFIG_GET()->output.port, true);
	TestConnectionChannel filter1(
	    TestConnectionChannel::Client, CONFIG_GET()->input.ip, CONFIG_GET()->input.port, true);
	TestConnectionChannel filter2(
	    TestConnectionChannel::Client, CONFIG_GET()->input.ip, CONFIG_GET()->input.port, true);
	TestConnectionChannel filter3(
	    TestConnectionChannel::Client, CONFIG_GET()->input.ip, CONFIG_GET()->input.port, true);

	int actionOrder = 0;
	static int currentStep = 0;

	// First connection => ok
	filter1.addCallback([&actionOrder, &filter2](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		ASSERT_GE(actionOrder, currentStep);
		ASSERT_LE(actionOrder, currentStep + 1);

		if(actionOrder == currentStep + 1) {
			currentStep += 2;
			filter2.start();
		}

		actionOrder++;

		Object::logStatic(Object::LL_Info, "", "Filter1 connected\n");
	});

	fakeServer.addCallback(
	    [&actionOrder, &filter2](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		    ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		    ASSERT_GE(actionOrder, 0);
		    ASSERT_LE(actionOrder, 1);

		    if(actionOrder == currentStep + 1) {
			    currentStep += 2;
			    filter2.start();
		    }

		    actionOrder++;

		    Object::logStatic(Object::LL_Info, "", "Fakeserver connected for filter1\n");
	    });

	// Second connection at once => ok
	filter2.addCallback([&actionOrder, &filter3](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		ASSERT_GE(actionOrder, currentStep);
		ASSERT_LE(actionOrder, currentStep + 1);

		if(actionOrder == currentStep + 1) {
			currentStep += 2;
			filter3.start();
		}

		actionOrder++;
		Object::logStatic(Object::LL_Info, "", "Filter2 connected\n");
	});

	fakeServer.addCallback(
	    [&actionOrder, &filter3](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		    ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		    ASSERT_GE(actionOrder, currentStep);
		    ASSERT_LE(actionOrder, currentStep + 1);

		    if(actionOrder == currentStep + 1) {
			    currentStep += 2;
			    filter3.start();
		    }

		    actionOrder++;

		    Object::logStatic(Object::LL_Info, "", "Fakeserver connected for filter2\n");
	    });

	// Third connection at once => rejected because 2 already connected (but still counted for connection per day)
	filter3.addCallback([&actionOrder](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		ASSERT_EQ(currentStep, actionOrder);
		actionOrder++;
		currentStep++;
		Object::logStatic(Object::LL_Info, "", "Filter3 connected\n");
	});

	filter3.addCallback(
	    [&actionOrder, &filter1, &filter2](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		    ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
		    ASSERT_EQ(currentStep, actionOrder);
		    actionOrder++;
		    currentStep++;

		    filter1.closeSession();
		    filter2.closeSession();
		    Object::logStatic(Object::LL_Info, "", "Filter3 disconnected\n");
	    });

	filter1.addCallback([&actionOrder, &filter1](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
		ASSERT_GE(actionOrder, currentStep);
		ASSERT_LE(actionOrder, currentStep + 3);

		if(actionOrder == currentStep + 3) {
			currentStep += 4;
			filter1.start();
		}

		actionOrder++;

		Object::logStatic(Object::LL_Info, "", "Filter1 disconnected\n");
	});

	filter2.addCallback([&actionOrder, &filter1](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
		ASSERT_GE(actionOrder, currentStep);
		ASSERT_LE(actionOrder, currentStep + 3);

		if(actionOrder == currentStep + 3) {
			currentStep += 4;
			filter1.start();
		}

		actionOrder++;

		Object::logStatic(Object::LL_Info, "", "Filter2 disconnected\n");
	});

	fakeServer.addCallback(
	    [&actionOrder, &filter1](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		    ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
		    ASSERT_GE(actionOrder, currentStep);
		    ASSERT_LE(actionOrder, currentStep + 3);

		    if(actionOrder == currentStep + 3) {
			    currentStep += 4;
			    filter1.start();
		    }

		    actionOrder++;

		    Object::logStatic(Object::LL_Info, "", "Fakeserver disconnected\n");
	    });

	fakeServer.addCallback(
	    [&actionOrder, &filter1](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		    ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
		    ASSERT_GE(actionOrder, currentStep);
		    ASSERT_LE(actionOrder, currentStep + 3);

		    if(actionOrder == currentStep + 3) {
			    currentStep += 4;
			    filter1.start();
		    }

		    actionOrder++;

		    Object::logStatic(Object::LL_Info, "", "Fakeserver disconnected\n");
	    });

	// 4th connection => ok (no other connection, 4 max per day)
	filter1.addCallback([&actionOrder, &filter1](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		ASSERT_GE(actionOrder, currentStep);
		ASSERT_LE(actionOrder, currentStep + 1);

		if(actionOrder == currentStep + 1) {
			currentStep += 2;
			filter1.closeSession();
		}

		actionOrder++;

		Object::logStatic(Object::LL_Info, "", "Filter1 connected\n");
	});

	fakeServer.addCallback(
	    [&actionOrder, &filter1](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		    ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		    ASSERT_GE(actionOrder, currentStep);
		    ASSERT_LE(actionOrder, currentStep + 1);

		    if(actionOrder == currentStep + 1) {
			    currentStep += 2;
			    filter1.closeSession();
		    }

		    actionOrder++;

		    Object::logStatic(Object::LL_Info, "", "Fakeserver connected\n");
	    });

	filter1.addCallback([&actionOrder, &filter1](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
		ASSERT_GE(actionOrder, currentStep);
		ASSERT_LE(actionOrder, currentStep + 1);

		if(actionOrder == currentStep + 1) {
			currentStep += 2;
			filter1.start();
		}

		actionOrder++;

		Object::logStatic(Object::LL_Info, "", "Filter1 disconnected\n");
	});

	fakeServer.addCallback(
	    [&actionOrder, &filter1](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		    ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
		    ASSERT_GE(actionOrder, currentStep);
		    ASSERT_LE(actionOrder, currentStep + 1);

		    if(actionOrder == currentStep + 1) {
			    currentStep += 2;
			    filter1.start();
		    }

		    actionOrder++;

		    Object::logStatic(Object::LL_Info, "", "Fakeserver disconnected\n");
	    });

	// 5th connection => KO (4 max per day)
	filter1.addCallback([&actionOrder](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		ASSERT_EQ(TestConnectionChannel::Event::Connection, event.type);
		ASSERT_EQ(currentStep, actionOrder);
		actionOrder++;
		currentStep++;
		Object::logStatic(Object::LL_Info, "", "Filter1 connected\n");
	});
	filter1.addCallback(
	    [&actionOrder, &fakeServer](TestConnectionChannel* channel, TestConnectionChannel::Event event) {
		    ASSERT_EQ(TestConnectionChannel::Event::Disconnection, event.type);
		    ASSERT_EQ(currentStep, actionOrder);
		    actionOrder++;
		    currentStep++;
		    Object::logStatic(Object::LL_Info, "", "Filter1 disconnected\n");
		    fakeServer.closeSession();
	    });

	test.addChannel(&fakeServer);
	fakeServer.start();
	test.addChannel(&filter1);
	test.addChannel(&filter2);
	test.addChannel(&filter3);
	filter1.start();
	test.run();
}
