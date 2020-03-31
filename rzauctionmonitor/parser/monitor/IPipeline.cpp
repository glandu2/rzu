#include "IPipeline.h"
#include "Console/ConsoleCommands.h"
#include "Core/PrintfFormats.h"
#include "Core/Utils.h"

std::vector<PipelineStepMonitor*> PipelineStepMonitor::monitors;

void PipelineStepMonitor::init() {
	ConsoleCommands::get()->addCommand(
	    "pipeline.stats", "stats", 0, 0, &PipelineStepMonitor::commandDumpStats, "Dump pipeline step stats");
	ConsoleCommands::get()->addCommand(
	    "pipeline.reset",
	    std::string(),
	    0,
	    1,
	    &PipelineStepMonitor::commandResetStats,
	    "Reset pipeline step stats",
	    "pipeline.reset [pipeline_name]: reset stats of the given pipeline step or all pipeline steps");
}

PipelineStepMonitor::PipelineStepMonitor() : running(0) {
	resetStats();
	monitors.push_back(this);
}

PipelineStepMonitor::~PipelineStepMonitor() {
	monitors.erase(std::remove(monitors.begin(), monitors.end(), this));
}

float PipelineStepMonitor::getActiveToIdleTimeRatio() {
	updateSumTime();
	return (float) sumOfActiveTime / (sumOfActiveTime + sumOfIdleTime);
}

float PipelineStepMonitor::getItemPerSecond() {
	updateSumTime();
	return (float) processedInputs / (sumOfActiveTime + sumOfIdleTime) * 1000.0f;
}

void PipelineStepMonitor::resetStats() {
	sumOfIdleTime = 0;
	sumOfActiveTime = 0;
	lastStateChangeTime = Utils::getTimeInMsec();
	processedInputs = 0;
}

void PipelineStepMonitor::notifyNewWorkRunning(size_t inputsNumber) {
	updateSumTime();
	running++;
	processedInputs += inputsNumber;
}

void PipelineStepMonitor::notifyWorkDone() {
	updateSumTime();
	running--;
}

void PipelineStepMonitor::updateSumTime() {
	uint64_t currentTime = Utils::getTimeInMsec();
	if(running) {
		sumOfActiveTime += currentTime - lastStateChangeTime;
	} else {
		sumOfIdleTime += currentTime - lastStateChangeTime;
	}
	lastStateChangeTime = currentTime;
}

void PipelineStepMonitor::commandDumpStats(IWritableConsole* console, const std::vector<std::string>& args) {
	size_t classNameMaxSize = 0;

	for(PipelineStepMonitor* monitor : monitors) {
		if(classNameMaxSize < monitor->getClassNameSize())
			classNameMaxSize = monitor->getClassNameSize();
	}

	for(PipelineStepMonitor* monitor : monitors) {
		console->writef("%*s: current input queue: %6d, block size: %4d, running works: %2d, active time ratio: %5.1f, "
		                "item per second: "
		                "%10.3f, processed items: %" PRIu64 "\r\n",
		                (int) classNameMaxSize,
		                monitor->getClassName(),
		                (int) monitor->getInputQueueSize(),
		                (int) monitor->getBlockSize(),
		                (int) monitor->running,
		                (double) monitor->getActiveToIdleTimeRatio() * 100.0,
		                (double) monitor->getItemPerSecond(),
		                (uint64_t) monitor->processedInputs);
	}
}

void PipelineStepMonitor::commandResetStats(IWritableConsole* console, const std::vector<std::string>& args) {
	for(PipelineStepMonitor* monitor : monitors) {
		if(args.size() < 1 || monitor->getClassName() == args[0]) {
			monitor->resetStats();
			console->writef("%s: stats reseted\r\n", monitor->getClassName());
		}
	}
}
