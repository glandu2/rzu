#include "Core/Timer.h"
#include "IPipeline.h"
#include <math.h>
#include <stdlib.h>
#include <string>
#include <time.h>

template<class T> void schedule_timeout(T callback, int delayms) {
	uv_timer_t* timer = new uv_timer_t;
	struct CallbackFunction {
		CallbackFunction(T& callback) : callback(std::move(callback)) {}

		void operator()() { callback(); }

	private:
		T callback;
	};

	timer->data = new CallbackFunction(callback);
	uv_timer_init(EventLoop::getLoop(), timer);
	uv_timer_start(
	    timer,
	    [](uv_timer_t* timer) {
		    CallbackFunction* function = static_cast<CallbackFunction*>(timer->data);
		    (*function)();
		    delete function;
		    uv_close((uv_handle_t*) timer, [](uv_handle_t* timer) { delete(uv_timer_t*) timer; });
	    },
	    delayms,
	    delayms);
}

// 1MiB data size
struct Data {
	Data(int d) : data(d) {}
	Data(Data&& other) {
		data = other.data;
		other.data = -1;
	}

	int data;
	char payload[1024 * 1024 - 4];

	Data(const Data&) = delete;
	Data& operator=(const Data&) = delete;
};

using WorkData = std::unique_ptr<Data>;

class Sleeper : public PipelineStep<WorkData, WorkData> {
public:
	using typename PipelineStep<WorkData, WorkData>::WorkItem;

	Sleeper(int maxSize, std::string name, int waitDelaySecond)
	    : PipelineStep<WorkData, WorkData>(maxSize, 10), name(name), waitDelaySecond(waitDelaySecond) {
		printf("%s: task duration: %d ms\n", name.c_str(), waitDelaySecond);
	}

	virtual void doWork(std::shared_ptr<WorkItem> item) override {
		printf("%s: start work on %d\n", name.c_str(), item->getSource()->data);
		schedule_timeout(
		    [this, item]() mutable {
			    this->addResult(item, std::make_unique<Data>(item->getSource()->data * 2));
			    // this->addResult(item, std::make_unique<Data>(item->getSource()->data * 3), item->getSource()->data *
			    // 3);

			    printf("%s: done %d -> %d / %d\n",
			           name.c_str(),
			           item->getSource()->data,
			           item->getSource()->data * 2,
			           item->getSource()->data * 3);
			    this->workDone(item);
		    },
		    waitDelaySecond);
	}

private:
	const std::string name;
	const int waitDelaySecond;
};

class Aggregator : public PipelineStep<WorkData, void> {
public:
	using typename PipelineStep<WorkData, void>::WorkItem;

	Aggregator(int maxSize, std::string name, int waitDelaySecond)
	    : PipelineStep<WorkData, void>(maxSize), name(name), waitDelaySecond(waitDelaySecond) {
		result = 0;
		printf("%s: task duration: %d ms\n", name.c_str(), waitDelaySecond);
	}

	virtual bool doCheckMergeInput(const std::vector<WorkData>& input) override { return input.size() != 1; }

	virtual void doWork(std::shared_ptr<WorkItem> item) override {
		printf("%s: start work on:\n", name.c_str());
		for(size_t i = 0; i < item->getSourceSize(); i++) {
			printf("  - %d\n", item->getSource(i)->data);
		}
		schedule_timeout(
		    [this, item]() mutable {
			    int output = 0;

			    for(size_t i = 0; i < item->getSourceSize(); i++) {
				    output += item->getSource(i)->data;
			    }

			    result += output;

			    printf("%s: done %d -> %d (commit data: %d)\n", name.c_str(), item->getSource()->data, output, output);
			    this->workDone(item);
		    },
		    waitDelaySecond);
	}

	static int result;

private:
	const std::string name;
	const int waitDelaySecond;
};

static int doTest() {
	std::vector<Sleeper> sleepers;

	for(size_t i = 0; i < 10; i++) {
		sleepers.emplace_back(2, std::string("Pipeline") + std::to_string(i + 1), rand() % 1000 + 1);
	}

	for(size_t i = 1; i < sleepers.size(); i++) {
		sleepers[i - 1].plug(&sleepers[i]);
	}

	Aggregator agg{2, "Aggregator", 10000};

	sleepers.back().plug(&agg);

	//	Sleeper p1{2, "Pipeline1", 445};
	//	Sleeper p2{2, "Pipeline2", 966};
	//	Sleeper p3{2, "Pipeline3", 789};
	//	Aggregator p4{2, "Aggregator", 846};

	int expectedResult = 0;

	for(int i = 0; i < 50; i++) {
		sleepers.front().queue(WorkData(new Data{i}));
	}
	for(int i = 0; i < 50; i++) {
		expectedResult += i * (int) pow(2, sleepers.size());
	}

	EventLoop::getInstance()->run(UV_RUN_DEFAULT);

	if(Aggregator::result != expectedResult) {
		printf("Expected %d, got %d\n", expectedResult, Aggregator::result);
		return false;
	}

	return true;
}
int main() {
	srand(time(nullptr));
	doTest();
}

int Aggregator::result;
