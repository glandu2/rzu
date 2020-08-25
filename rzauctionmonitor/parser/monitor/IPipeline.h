#pragma once

#include "./Extern.h"
#include "Core/Object.h"
#include "Core/PrintfFormats.h"
#include "Core/Utils.h"
#include <algorithm>
#include <deque>
#include <memory>
#include <vector>

template<class T> class IPipelineProducer;

template<class T, class CopyableEnable = void> class IPipelineConsumer {
public:
	virtual ~IPipelineConsumer() {}
	virtual void queue(T data) = 0;
	virtual void cancel() = 0;
	virtual int64_t inputAvailable() = 0;

	virtual void setProducer(IPipelineProducer<T>* producer) = 0;
};

template<class T> class IPipelineProducer {
public:
	virtual ~IPipelineProducer() {}
	virtual void notifyError(int status) = 0;
	virtual void notifyOutputAvailable() = 0;

	template<class NextPipeline> NextPipeline* plug(NextPipeline* nextPipeline) {
		nextPipeline->setProducer(this);
		return nextPipeline;
	}
};

template<class SourceData, class OutputData> class PipelineWorkBase {
public:
	// PipelineWorkBase(SourceData&& sourceData) { inputs.push_back(std::move(sourceData)); }
	PipelineWorkBase(std::vector<SourceData>&& sourceData)
	    : inputs(std::move(sourceData)), workStartTime(Utils::getTimeInMsec()) {}

	PipelineWorkBase(const PipelineWorkBase&) = delete;
	PipelineWorkBase& operator=(const PipelineWorkBase&) = delete;

	SourceData& getSource(size_t index = 0) { return inputs.at(index); }
	size_t getSourceSize() { return inputs.size(); }
	std::vector<SourceData>& getSources() { return inputs; }

	void cancelWork() { canceled = true; }
	bool isCanceled() { return canceled; }

	void setName(const std::string& name) { workName = name; }
	const std::string& getName() { return workName; }
	uint64_t getWorkDuration() { return Utils::getTimeInMsec() - workStartTime; }

private:
	std::vector<SourceData> inputs;
	bool canceled = false;
	std::string workName;
	uint64_t workStartTime;
};

template<class SourceData, class OutputData, class Enabler = void>
class PipelineWork : public PipelineWorkBase<SourceData, OutputData> {
	using PipelineWorkBase<SourceData, OutputData>::PipelineWorkBase;
};

template<class SourceData, class OutputData>
class PipelineWork<SourceData, OutputData, std::enable_if_t<!std::is_void<OutputData>::value>>
    : public PipelineWorkBase<SourceData, OutputData> {
public:
	using PipelineWorkBase<SourceData, OutputData>::PipelineWorkBase;

	template<class T> void addOutput(T&& output) { outputs.emplace_back(std::forward<T>(output)); }

	void queueToNext(IPipelineConsumer<OutputData>* next) {
		std::vector<OutputData> outputsToSend = std::move(outputs);
		outputs.clear();

		for(auto&& output : outputsToSend) {
			next->queue(std::move(output));
		}
	}

private:
	std::vector<OutputData> outputs;
};

class IWritableConsole;

class RZAUCTIONWATCHER_EXTERN PipelineStepMonitor : public Object {
public:
	static void init();

	PipelineStepMonitor();
	virtual ~PipelineStepMonitor();
	PipelineStepMonitor(const PipelineStepMonitor&) = default;
	PipelineStepMonitor& operator=(const PipelineStepMonitor&) = default;

protected:
	virtual size_t getInputQueueSize() = 0;
	virtual size_t getBlockSize() = 0;
	virtual size_t getMaxInputQueueSize() = 0;
	virtual size_t getMaxParallelWork() = 0;
	float getActiveToIdleTimeRatio();
	float getItemPerSecond();
	void resetStats();

	void notifyNewWorkRunning(size_t inputsNumber);
	void notifyWorkDone();
	size_t getRunningWorkNumber() { return running; }

private:
	void updateSumTime();

private:
	size_t running;
	uint64_t sumOfIdleTime;
	uint64_t sumOfActiveTime;
	uint64_t lastStateChangeTime;
	size_t processedInputs;

private:
	static void commandDumpStats(IWritableConsole* console, const std::vector<std::string>& args);
	static void commandDumpIncrementalStats(IWritableConsole* console, const std::vector<std::string>& args);
	static void commandResetStats(IWritableConsole* console, const std::vector<std::string>& args);

private:
	static std::vector<PipelineStepMonitor*> monitors;
};

template<class Input, class Output, class Enabler = void> class PipelineStep;

template<class Input, class Output>
class PipelineStepBase : public PipelineStepMonitor, public IPipelineConsumer<Input> {
public:
	using WorkItem = PipelineWork<Input, Output>;

	PipelineStepBase(size_t maxSize, size_t maxParallelWork = 1, size_t maxBlockSize = 1)
	    : maxSize(maxSize), maxParallelWork(maxParallelWork), maxBlockSize(maxBlockSize), prev(nullptr) {}

	PipelineStepBase(PipelineStepBase&&) = default;
	PipelineStepBase& operator=(PipelineStepBase&&) = default;
	~PipelineStepBase() = default;
	PipelineStepBase(const PipelineStepBase&) = delete;
	PipelineStepBase& operator=(const PipelineStepBase&) = delete;

	virtual size_t getInputQueueSize() { return inputQueue.size(); }
	virtual size_t getMaxInputQueueSize() { return maxSize; }
	virtual size_t getBlockSize() { return maxBlockSize; }
	virtual size_t getMaxParallelWork() { return maxParallelWork; }

	virtual int64_t inputAvailable() override {
		return static_cast<int64_t>(maxSize) - static_cast<int64_t>(getRunningWorkNumber()) -
		       static_cast<int64_t>(inputQueue.size());
	}

	virtual void queue(Input inputToMove) override {
		inputQueue.emplace_back(std::move(inputToMove));
		doNextWork();
	}

	virtual void cancel() override {
		inputQueue.clear();
		pendingNextWorkQueue.clear();
		for(const std::shared_ptr<WorkItem>& work : workInProgress)
			work->cancelWork();
		for(const std::shared_ptr<WorkItem>& work : workInProgress)
			doCancelWork(work);
	}

	void notifyError(int status) {
		if(prev)
			prev->notifyError(status);
	}

	void clear() { inputQueue.clear(); }

	virtual void setProducer(IPipelineProducer<Input>* producer) override { prev = producer; }

protected:
	virtual bool doCheckMergeInput(const std::vector<Input>& pendingInputs) {
		if(inputQueue.size() > 0 && pendingInputs.size() < maxBlockSize)
			return true;
		else
			return false;
	}

	virtual void doWork(std::shared_ptr<WorkItem> item) = 0;

	virtual void doCancelWork(std::shared_ptr<WorkItem> item) {}

	void workDone(std::shared_ptr<WorkItem> item, int status) {
		if(status)
			log(LL_Error, "%s: Work done with status %d\n", item->getName().c_str(), status);
		auto it = std::remove(workInProgress.begin(), workInProgress.end(), item);
		if(it < workInProgress.end())
			workInProgress.erase(it);
		notifyWorkDone();
	}

	void doNextWork() {
		while(getRunningWorkNumber() < maxParallelWork && !isNextFull() && !inputQueue.empty()) {
			bool mergeInput;
			do {
				pendingNextWorkQueue.push_back(std::move(inputQueue.front()));
				inputQueue.pop_front();
				mergeInput = doCheckMergeInput(pendingNextWorkQueue);
			} while(!inputQueue.empty() && mergeInput);

			if(!mergeInput) {
				notifyNewWorkRunning(pendingNextWorkQueue.size());
				std::shared_ptr<WorkItem> work(new WorkItem(std::move(pendingNextWorkQueue)));
				pendingNextWorkQueue.clear();
				workInProgress.push_back(work);
				doWork(std::move(work));
			}
		}

		if(inputAvailable() > 0 && prev) {
			prev->notifyOutputAvailable();
		}
	}

private:
	virtual bool isNextFull() = 0;

private:
	friend class PipelineStep<Input, Output>;

	const size_t maxSize;
	const size_t maxParallelWork;
	const size_t maxBlockSize;
	IPipelineProducer<Input>* prev;
	std::deque<Input> inputQueue;
	std::vector<Input> pendingNextWorkQueue;
	std::vector<std::shared_ptr<WorkItem>> workInProgress;
};

template<class Input, class Output, class Enabler> class PipelineStep : public PipelineStepBase<Input, Output> {
public:
	using PipelineStepBase<Input, Output>::PipelineStepBase;
	using typename PipelineStepBase<Input, Output>::WorkItem;

protected:
	void workDone(std::shared_ptr<WorkItem> item, int status = 0) {
		PipelineStepBase<Input, Output>::workDone(item, status);
		if(!item->isCanceled() && !status)
			PipelineStepBase<Input, Output>::doNextWork();
		else if(status)
			PipelineStepBase<Input, Output>::notifyError(status);
	}

private:
	virtual bool isNextFull() final { return false; }
};

template<class Input, class Output>
class PipelineStep<Input, Output, std::enable_if_t<!std::is_void<Output>::value>>
    : public PipelineStepBase<Input, Output>, public IPipelineProducer<Output> {
public:
	using typename PipelineStepBase<Input, Output>::WorkItem;

	PipelineStep(size_t maxSize, size_t maxParallelWork = 1, size_t maxBlockSize = 1)
	    : PipelineStepBase<Input, Output>(maxSize, maxParallelWork, maxBlockSize), next(nullptr) {}

	template<class NextPipeline> NextPipeline* plug(NextPipeline* nextPipeline) {
		setConsumer(nextPipeline);
		return IPipelineProducer<Output>::plug(nextPipeline);
	}

	virtual int64_t inputAvailable() override {
		if(next)
			return PipelineStepBase<Input, Output>::inputAvailable() + next->inputAvailable();
		else
			return PipelineStepBase<Input, Output>::inputAvailable();
	}

	virtual void cancel() override {
		PipelineStepBase<Input, Output>::cancel();
		if(next)
			next->cancel();
	}

	virtual void notifyError(int status) override { PipelineStepBase<Input, Output>::notifyError(status); }

	virtual void notifyOutputAvailable() override { PipelineStepBase<Input, Output>::doNextWork(); }
	void setConsumer(IPipelineConsumer<Output>* consumer) { next = consumer; }

protected:
	template<class T> void addResult(std::shared_ptr<WorkItem>& item, T&& output) {
		item->addOutput(std::forward<T>(output));
	}

	void workDone(std::shared_ptr<WorkItem> item, int status = 0) {
		PipelineStepBase<Input, Output>::workDone(item, status);
		if(!item->isCanceled() && !status) {
			if(next)
				item->queueToNext(next);
			PipelineStepBase<Input, Output>::doNextWork();
		} else if(status) {
			notifyError(status);
		}
	}

private:
	virtual bool isNextFull() final { return next && next->inputAvailable() <= 0; }

private:
	IPipelineConsumer<Output>* next;
};
