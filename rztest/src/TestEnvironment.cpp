#include "TestEnvironment.h"
#include "Core/EventLoop.h"
#include "Terminator.h"
#include "TestGlobalConfig.h"
#include "WaitConnectionOpen.h"
#include "uv.h"
#include <stdarg.h>

TestProcessBase::~TestProcessBase() {
	for(size_t i = 0; i < processes.size(); i++) {
		delete processes[i];
	}
}

void TestProcessBase::SetUp() {
	doKillAll = true;
}

void TestProcessBase::TearDown() {
	Timer<TestProcessBase> timer;

	if(TestGlobalConfig::get()->enableExecutableSpawn.get()) {
		for(size_t i = 0; i < processes.size(); i++) {
			if(doKillAll)
#ifdef WIN32
				uv_process_kill(processes[i], SIGINT);
#else
				uv_process_kill(processes[i], SIGTERM);
#endif

			uv_ref((uv_handle_t*) processes[i]);
		}

		if(!doKillAll) {
			timer.start(this, &TestProcessBase::tearDownTimeout, 5000, 0);
			timer.unref();
		}
		EventLoop::getInstance()->run(UV_RUN_DEFAULT);
		if(!doKillAll) {
			timer.stop();
		}

		for(size_t i = 0; i < processes.size(); i++) {
			processes[i]->data = nullptr;
		}
	}
}

void TestProcessBase::vspawnProcess(int portCheck, const char* exeFile, int argNumber, va_list argsList) {
	if(TestGlobalConfig::get()->enableExecutableSpawn.get() == false) {
		log(LL_Info, "Ignoring spawn process request: config disabled process spawning\n");
		return;
	}

	uv_process_options_t options = {0};
	std::vector<char*> args;
	uv_stdio_container_t stdioInherit[3];
	uv_process_t* handle = new uv_process_t;

	args.push_back((char*) exeFile);

	for(int i = 0; i < argNumber; i++) {
		char* arg = va_arg(argsList, char*);
		if(arg == nullptr)
			break;
		args.push_back(arg);
	}
	args.push_back(nullptr);

	options.exit_cb = &executableExited;
	options.file = exeFile;
	options.args = &args[0];
	options.flags = 0;

	for(int i = 0; i < 3; i++) {
		stdioInherit[i].flags = UV_IGNORE;
		stdioInherit[i].data.fd = i;
	}

	options.stdio = stdioInherit;
	options.stdio_count = 3;

	int result = uv_spawn(EventLoop::getLoop(), handle, &options);
	ASSERT_EQ(0, result) << "Error while starting process " << exeFile << " : " << uv_strerror(result) << "(" << result
	                     << ")\n";

	log(LL_Info, "Started process %s with pid %d\n", exeFile, handle->pid);
	handle->data = this;
	processes.push_back(handle);

	uv_unref((uv_handle_t*) handle);

	WaitConnectionOpen checkConnection;
	checkConnection.start(portCheck, 30000);
	EventLoop::getInstance()->run(UV_RUN_DEFAULT);

	ASSERT_EQ(true, checkConnection.isOpen()) << "Error while waiting process " << exeFile << " : waiting timeout\n";
}

void TestProcessBase::spawnProcess(int portCheck, const char* exeFile, int argNumber, ...) {
	va_list argsList;
	va_start(argsList, argNumber);
	vspawnProcess(portCheck, exeFile, argNumber, argsList);
	va_end(argsList);
}

void TestProcessBase::stop(int port) {
	if(TestGlobalConfig::get()->enableExecutableSpawn.get() == false) {
		log(LL_Debug, "Ignoring stop process request: config disabled process spawning\n");
		return;
	}

	doKillAll = false;

	Terminator terminator;
	terminator.start("127.0.0.1", port);
	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}

void TestProcessBase::executableExited(uv_process_t* process, int64_t exit_status, int) {
	TestProcessBase* thisInstance = (TestProcessBase*) process->data;
	Object::Level level = Object::LL_Error;
	if((thisInstance && thisInstance->doKillAll && exit_status == 1) || exit_status == 0)
		level = Object::LL_Info;

	Object::logStatic(level, "TestProcessBase", "Process %d exited with value %ld\n", process->pid, (long) exit_status);
	uv_close((uv_handle_t*) process, nullptr);
}

void TestProcessBase::tearDownTimeout() {
	for(size_t i = 0; i < processes.size(); i++) {
		uv_process_kill(processes[i], SIGKILL);
	}
}

TestEnvironment::~TestEnvironment() {}
