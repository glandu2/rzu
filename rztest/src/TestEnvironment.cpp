#include "TestEnvironment.h"
#include "uv.h"
#include <stdarg.h>
#include "Core/EventLoop.h"
#include "Terminator.h"
#include "WaitConnectionOpen.h"
#include "TestGlobalConfig.h"

TestEnvironment::~TestEnvironment()
{
	for(size_t i = 0; i < processes.size(); i++) {
		delete processes[i];
	}
}

void TestEnvironment::SetUp() {
	doKillAll = true;
	beforeTests();
}

void TestEnvironment::TearDown()
{
	Timer<TestEnvironment> timer;

	afterTests();

	if(TestGlobalConfig::get()->enableExecutableSpawn.get()) {
		for(size_t i = 0; i < processes.size(); i++) {
			if(doKillAll)
				uv_process_kill(processes[i], SIGTERM);
			uv_ref((uv_handle_t*)processes[i]);
		}

		if(!doKillAll) {
			timer.start(this, &TestEnvironment::tearDownTimeout, 5000, 0);
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

void TestEnvironment::spawnProcess(int portCheck, const char* exeFile, int argNumber, ...) {
	if(TestGlobalConfig::get()->enableExecutableSpawn.get() == false) {
		log(LL_Info, "Ignoring spawn process request: config disabled process spawning\n");
		return;
	}

	uv_process_options_t options = {0};
	std::vector<char*> args;
	uv_stdio_container_t stdioInherit[3];
	va_list argsList;
	uv_process_t* handle = new uv_process_t;

	args.push_back((char*)exeFile);

	va_start(argsList, argNumber);
	for(int i = 0; i < argNumber; i++) {
		char* arg = va_arg(argsList, char*);
		if(arg == nullptr)
			break;
		args.push_back(arg);
	}
	va_end(argsList);
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
	ASSERT_EQ(0, result) << "Error while starting process " << exeFile << " : " << uv_strerror(result) << "(" << result << ")\n";

	log(LL_Info, "Started process %s with pid %d\n", exeFile, handle->pid);
	handle->data = this;
	processes.push_back(handle);

	uv_unref((uv_handle_t*)handle);

	WaitConnectionOpen checkConnection;
	checkConnection.start(portCheck, 30000);
	EventLoop::getInstance()->run(UV_RUN_DEFAULT);

	ASSERT_EQ(true, checkConnection.isOpen()) << "Error while waiting process " << exeFile << " : waiting timeout\n";
}

void TestEnvironment::stop(int port)
{
	if(TestGlobalConfig::get()->enableExecutableSpawn.get() == false) {
		log(LL_Debug, "Ignoring stop process request: config disabled process spawning\n");
		return;
	}

	doKillAll = false;

	Terminator terminator;
	terminator.start("127.0.0.1", port);
	EventLoop::getInstance()->run(UV_RUN_DEFAULT);
}

void TestEnvironment::executableExited(uv_process_t* process, int64_t exit_status, int) {
	TestEnvironment* thisInstance = (TestEnvironment*)process->data;
	Object::Level level = Object::LL_Error;
	if((thisInstance && thisInstance->doKillAll && exit_status == 1) || exit_status == 0)
		level = Object::LL_Info;

	Object::logStatic(level, "TestEnvironment", "Process %d exited with value %ld\n", process->pid, (long)exit_status);
	uv_close((uv_handle_t*)process, nullptr);
}

void TestEnvironment::tearDownTimeout() {
	for(size_t i = 0; i < processes.size(); i++) {
		uv_process_kill(processes[i], SIGKILL);
	}
}
