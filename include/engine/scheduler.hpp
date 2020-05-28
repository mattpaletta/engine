#pragma once

#if ENGINE_ENABLE_MULTITHREADED
	#include <ftl/atomic_counter.h>
	#include <ftl/task_scheduler.h>
#endif

#include <vector>
#include <functional>

class Scheduler {
private:
#if ENGINE_ENABLE_MULTITHREADED
	ftl::TaskScheduler taskScheduler;
#endif

public:
	Scheduler();
	~Scheduler();

	// Not copyable
	Scheduler(const Scheduler&) = delete;
	Scheduler& operator=(const Scheduler&) = delete;
};
