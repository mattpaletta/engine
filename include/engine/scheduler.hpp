#pragma once

#include <ftl/atomic_counter.h>
#include <ftl/task_scheduler.h>
#include <vector>
#include <functional>

class Scheduler {
private:
	ftl::TaskScheduler taskScheduler;

public:
	Scheduler();
	~Scheduler();

	// Not copyable
	Scheduler(const Scheduler&) = delete;
	Scheduler& operator=(const Scheduler&) = delete;
};