#ifndef IPIPELINE_H
#define IPIPELINE_H

#include <errno.h>
#include <functional>
#include <memory>
#include <system_error>
#include <unordered_map>
#include <unordered_set>
#include <vector>

template<class T> class IPipelineProducer;
template<class Output> class IPipelineOutput;

template<class Output> class ICommitable {
public:
	virtual ~ICommitable() {}
	virtual void commit(std::unique_ptr<IPipelineOutput<Output>> item, int status) = 0;
};

template<class Output> class IPipelineWork {
public:
	virtual ~IPipelineWork() {}
};

template<class Output> class IPipelineOutput {
public:
	virtual ~IPipelineOutput() {}
	virtual ICommitable<Output>* getCommiter() = 0;
	virtual const Output& getData() const = 0;
};

template<class T> class IPipelineConsumer {
public:
	virtual ~IPipelineConsumer() {}
	virtual void queue(std::unique_ptr<IPipelineOutput<T>> originator) = 0;
	virtual void queue(T data, ICommitable<T>* commiter = nullptr) = 0;
	virtual void cancel() = 0;
	virtual bool isFull() = 0;

	virtual void setProducer(IPipelineProducer<T>* producer) = 0;
};

template<class T> class IPipelineProducer {
public:
	virtual ~IPipelineProducer() {}
	virtual void notifyOutputAvailable() = 0;

	virtual void setConsumer(IPipelineConsumer<T>* consumer) = 0;
};

// template<class SourceData> struct SourceIterator {
//	SourceIterator(typename std::vector<PipelineSource>::iterator it) : it(it) {}

//	SourceData& operator*() { return it->get(); }
//	SourceData& operator++() {
//		++it;
//		return *this;
//	}

// private:
//	typename std::vector<PipelineSource>::iterator it;
//};

// template<class SourceData> struct Sources {
//	Sources(std::vector<Source>& sources) : sources(sources) {}

//	SourceIterator begin() { return SourceIterator(sources.begin()); }
//	SourceIterator end() { return SourceIterator(sources.end()); }

// private:
//	std::vector<Source>& sources;
//};

/*
 * Pending input in queue: PipelineSourceRef<Input>
 * input being worked on: PipelineSourceRef<Input>
 * result generated from work: PipelineOutput{PipelineSourceRef<Input>, CommitData}, Output
 * pending commit: PipelineOutput{PipelineSourceRef<Input>, CommitData}
 * pending commit done: PipelineOutput{PipelineSourceRef<Input>, CommitData}
 * commit to upstream: PipelineSourceRef<Input>
 *    How to manage ICommitable memory scope (destroy)
 */

/* Data model:
 * N {input, commitTarget} <=> 1 work <=> M {output, commitData}
 *
 * work is done when all outputs are commited
 * when work is done, all input are commited using commitTarget
 * when commitTarget is triggered, upstream step's output is commited using commitData
 *
 * work is:
 *  - function to merge inputs and trigger work if no more input is expected
 *    - enum { MergeMore, DoWork } mergeInputs(input/commitTarget)
 *  - function to do the actual work on inputs
 *    - void doWork(inputs)
 *  - function to generate a work result === an output
 *    - void addResult(output, commitData)
 *  - function to finalize a work and start the next work
 *    - void workDone()
 *  - function commit to trigger commit all inputs with commitTarget
 *
 *  - 1 function to cancel work
 *    - void cancel()
 *      - call doCancelWork on active work(s)
 *      - flush input queue
 *      - outputs ? flushed by downstream step => call next->cancel()
 *
 * work cycle:
 *  - Pending: between queue and doWork
 *  - Processing: between doWork and workDone
 *  - PendingDownstreamSteps: between workDone and commit
 *  - Commiting: between commit and commitDone
 *
 * output is:
 *  - function to commit ouput
 *    - void commit(commitData)
 *  - function to finalize commit and trigger work.commit if it is the last output to be commited
 *    - void doCommit(commitData)
 *
 * input reference an output (loose coupling) via std::unique_ptr between queue and before commitDone
 * referenced output via input is released in commitDone stage
 * work is created before doWork and deleted after commitDone
 * work has composition ownership on inputs/commitTargets
 * work has shared_ptr ownership on outputs between addResult and workDone
 * outputs have shared_ptr ownership on work when doing workDone
 *   - If all outputs are deleted because of a cancel request, the work will be deleted too
 *
 * work's ownership is:
 *   - between queue and doWork: pendingInput vector
 *   - between doWork and workDone: doWork function
 *   - between workDone and doCommit: outputs
 *
 * input ownership is same as work (composition)
 *
 * output's ownership is:
 *   - between queue and addResult: not created yet
 *   - between addResult and workDone: parent work
 *   - between workDone and commit: downstream's input
 *   - between commit and doCommit: commit function
 */

template<class SourceData> using PipelineSourceRef = std::unique_ptr<IPipelineOutput<SourceData>>;

template<class SourceData, class OutputData> class PipelineWork;

template<class OutputData> class RawInput : public IPipelineOutput<OutputData> {
public:
	RawInput(OutputData outputData, ICommitable<OutputData>* commiter)
	    : outputData(std::move(outputData)), commiter(commiter) {}

	virtual ICommitable<OutputData>* getCommiter() { return commiter; }
	virtual const OutputData& getData() const override { return outputData; }

protected:
	OutputData outputData;
	ICommitable<OutputData>* commiter;
};

template<class SourceData, class OutputData, class CommitData, class CommitCallback>
class PipelineOutput : public IPipelineOutput<OutputData> {
public:
	PipelineOutput(std::shared_ptr<PipelineWork<SourceData, OutputData>> parentWork,
	               OutputData outputData,
	               CommitCallback* commitCallback,
	               CommitData commitData)
	    : parentWork(parentWork),
	      outputData(std::move(outputData)),
	      commitCallback(commitCallback),
	      commitData(std::move(commitData)) {}

	PipelineOutput(PipelineOutput&& other) noexcept {
		parentWork = std::move(other.parentWork);
		commitCallback = other.commitCallback;
		commitData = std::move(other.commitData);
		outputData = std::move(other.outputData);
		other.commitCallback = nullptr;
	}

	PipelineOutput(const PipelineOutput&) = delete;
	PipelineOutput& operator=(const PipelineOutput&) = delete;

	virtual ICommitable<OutputData>* getCommiter() { return commitCallback; }
	std::shared_ptr<PipelineWork<SourceData, OutputData>> getRealWork() { return parentWork; }

	virtual const OutputData& getData() const override { return outputData; }
	const CommitData& getCommitData() { return commitData; }

protected:
	std::shared_ptr<PipelineWork<SourceData, OutputData>> parentWork;
	OutputData outputData;
	CommitCallback* commitCallback;
	CommitData commitData;
	int commitStatus;
};

template<class SourceData, class OutputData> class PipelineWork : public IPipelineWork<OutputData> {
public:
	PipelineWork(PipelineSourceRef<SourceData>&& sourceData) { inputs.push_back(std::move(sourceData)); }
	PipelineWork(std::vector<PipelineSourceRef<SourceData>>&& sourceData) : inputs(std::move(sourceData)) {}

	PipelineWork(const PipelineWork&) = delete;
	PipelineWork& operator=(const PipelineWork&) = delete;

	const SourceData& getSource(size_t index = 0) { return inputs.at(index)->getData(); }
	size_t getSourceSize() { return inputs.size(); }

	void addOutput(std::unique_ptr<IPipelineOutput<OutputData>>& output) { outputs.push_back(std::move(output)); }
	bool hasOutput() { return !outputs.empty(); }

	void queueToNext(IPipelineConsumer<OutputData>* next) {
		std::vector<std::unique_ptr<IPipelineOutput<OutputData>>> outputsToSend = std::move(outputs);
		outputs.clear();

		for(std::unique_ptr<IPipelineOutput<OutputData>>& output : outputsToSend) {
			outputsPendingCommit.push_back(output.get());
		}

		for(std::unique_ptr<IPipelineOutput<OutputData>>& output : outputsToSend) {
			next->queue(std::move(output));
		}
	}

	std::vector<std::unique_ptr<IPipelineOutput<OutputData>>> queueToCommit() {
		std::vector<std::unique_ptr<IPipelineOutput<OutputData>>> outputsToSend = std::move(outputs);
		outputs.clear();

		for(std::unique_ptr<IPipelineOutput<OutputData>>& output : outputsToSend) {
			outputsPendingCommit.push_back(output.get());
		}

		return std::move(outputsToSend);
	}

	void cancelWork() { canceled = true; }
	bool isCanceled() { return canceled; }

	void commitOutput(std::unique_ptr<IPipelineOutput<OutputData>>& output, int status) {
		auto it = std::find(outputsPendingCommit.begin(), outputsPendingCommit.end(), output.get());
		if(it != outputsPendingCommit.end()) {
			outputsPendingCommit.erase(it);
		} else {
			printf("Output %p not pending in work %p\n", output.get(), this);
		}
		if(outputsPendingCommit.empty()) {
			for(PipelineSourceRef<SourceData>& input : inputs) {
				ICommitable<SourceData>* commiter = input->getCommiter();
				if(commiter)
					commiter->commit(std::move(input), status);
			}
			inputs.clear();
		}
	}

private:
	std::vector<PipelineSourceRef<SourceData>> inputs;
	std::vector<std::unique_ptr<IPipelineOutput<OutputData>>> outputs;
	std::vector<IPipelineOutput<OutputData>*> outputsPendingCommit;
	bool canceled = false;
};

template<class Input, class Output, class CommitData>
class PipelineStep : public IPipelineConsumer<Input>, public IPipelineProducer<Output>, public ICommitable<Output> {
public:
	using WorkItem = PipelineWork<Input, Output>;
	using CommitItem = PipelineOutput<Input, Output, CommitData, PipelineStep>;
	PipelineStep(size_t maxSize, size_t maxParallelWork = 1)
	    : maxSize(maxSize), maxParallelWork(maxParallelWork), prev(nullptr), next(nullptr), running(0) {}

	virtual bool isFull() override { return inputQueue.size() >= maxSize; }

	virtual void queue(std::unique_ptr<IPipelineOutput<Input>> commitCallback) override {
		inputQueue.emplace_back(std::move(commitCallback));
		doNextWork();
	}

	virtual void queue(Input sourceData, ICommitable<Input>* commiter = nullptr) override {
		inputQueue.emplace_back(new RawInput<Input>(std::move(sourceData), commiter));
		doNextWork();
	}

	virtual void cancel() override {
		inputQueue.clear();
		pendingNextWorkQueue.clear();
		for(const std::shared_ptr<WorkItem>& work : workInProgress)
			work->cancelWork();
		for(const std::shared_ptr<WorkItem>& work : workInProgress)
			doCancelWork(work);
		if(next)
			next->cancel();
	}

	virtual void commit(std::unique_ptr<IPipelineOutput<Output>> item, int status) override {
		doCommit(std::unique_ptr<CommitItem>(static_cast<CommitItem*>(item.release())), status);
	}

	template<class NextPipeline> NextPipeline* plug(NextPipeline* nextPipeline) {
		setConsumer(nextPipeline);
		nextPipeline->setProducer(this);
		return nextPipeline;
	}

	virtual void notifyOutputAvailable() override { doNextWork(); }

	virtual void setConsumer(IPipelineConsumer<Output>* consumer) override { next = consumer; }
	virtual void setProducer(IPipelineProducer<Input>* producer) override { prev = producer; }

protected:
	virtual bool doCheckMergeInput(const std::vector<PipelineSourceRef<Input>>&) { return false; }
	virtual void doWork(std::shared_ptr<WorkItem> item) = 0;

	void addResult(std::shared_ptr<WorkItem> item, Output workData, CommitData commitData) {
		std::unique_ptr<IPipelineOutput<Output>> output{
		    new CommitItem(item, std::move(workData), this, std::move(commitData))};
		item->addOutput(std::move(output));
	}

	void addResult(std::shared_ptr<WorkItem> item, Output workData) {
		std::unique_ptr<IPipelineOutput<Output>> output{new CommitItem(item, workData, this, workData)};
		item->addOutput(std::move(output));
	}

	virtual void doCancelWork(std::shared_ptr<WorkItem> item) {}

	void workDone(std::shared_ptr<WorkItem> item, int status) {
		workInProgress.erase(item);
		if(item->isCanceled() || !item->hasOutput() || status || !next) {
			std::vector<std::unique_ptr<IPipelineOutput<Output>>> outputs{std::move(item->queueToCommit())};
			for(std::unique_ptr<IPipelineOutput<Output>>& output : outputs) {
				commit(std::move(output), item->isCanceled() ? ECANCELED : status);
			}
		} else {
			item->queueToNext(next);
		}
		running--;
		doNextWork();
	}

	virtual void doCommit(std::unique_ptr<CommitItem>& item, int status) { commitDone(item, status); }

	void commitDone(std::unique_ptr<CommitItem>& item, int status) {
		std::shared_ptr<WorkItem> work = item->getRealWork();
		work->commitOutput(std::unique_ptr<IPipelineOutput<Output>>(item.release()), status);
		// input given back to their parent work (in previous step)
		// outputs destroyed in commitOutput
		// work destroyed here
	}

private:
	void doNextWork() {
		while(running < maxParallelWork && (!next || !next->isFull()) && !inputQueue.empty()) {
			bool mergeInput;
			do {
				pendingNextWorkQueue.push_back(std::move(inputQueue.front()));
				inputQueue.erase(inputQueue.begin());
				mergeInput = doCheckMergeInput(pendingNextWorkQueue);
			} while(!inputQueue.empty() && mergeInput);

			if(!mergeInput) {
				std::shared_ptr<WorkItem> work(new WorkItem(std::move(pendingNextWorkQueue)));
				pendingNextWorkQueue.clear();
				running++;
				workInProgress.insert(work);
				doWork(std::move(work));
			}

			if(!isFull() && prev) {
				prev->notifyOutputAvailable();
			}
		}
	}

private:
	size_t maxSize;
	size_t maxParallelWork;
	IPipelineProducer<Input>* prev;
	IPipelineConsumer<Output>* next;
	std::vector<PipelineSourceRef<Input>> inputQueue;
	std::vector<PipelineSourceRef<Input>> pendingNextWorkQueue;
	std::unordered_set<std::shared_ptr<WorkItem>> workInProgress;
	size_t running;
};

template<class Input, class Output, class CommitData, class Lambda, class CommitLambda>
class PipelineLambda : public PipelineStep<Input, Output, CommitData> {
public:
	using typename PipelineStep<Input, Output, CommitData>::Item;

	PipelineLambda(size_t maxSize, Lambda doWorkCallback, CommitLambda doCommitLambda)
	    : PipelineStep<Input, Output, CommitData>(maxSize),
	      doWorkCallback(doWorkCallback),
	      doCommitLambda(doCommitLambda) {}

protected:
	virtual void doWork(std::unique_ptr<Item> item) override { doWorkCallback(this, std::move(item)); }
	virtual void doCommit(std::unique_ptr<Item> item, int status) override {
		doCommitLambda(this, std::move(item), status);
	}

private:
	Lambda doWorkCallback;
	CommitLambda doCommitLambda;
};

template<class Input, class Output, class CommitData, class Lambda, class CommitLambda>
PipelineLambda<Input, Output, CommitData, Lambda, CommitLambda> create_pipeline(size_t maxSize,
                                                                                Lambda doWork,
                                                                                CommitLambda doCommit) {
	return PipelineLambda<Input, Output, CommitData, Lambda, CommitLambda>(maxSize, doWork, doCommit);
}

#endif
