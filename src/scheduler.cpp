#include "engine/scheduler.hpp"

Scheduler::Scheduler() {
	// Create the task scheduler and bind the main thread to it
#if ENGINE_ENABLE_MULTITHREADED
	taskScheduler.Init();
#endif
}

Scheduler::~Scheduler() {
}

/*
template<typename Seq, typename output>
std::vector<output> Scheduler::map(const std::function<output(Seq::value_type)>&& f, const Seq& data) {
	std::vector<output> out;
	out.reserve(funcs.size());
#if ENGINE_ENABLE_MULTITHREADED
	ftl::AtomicCounter counter(&this->taskScheduler);
	{
		std::vector<ftl::Task> tasks;
		for (std::size_t i = 0; i < funcs.size(); ++i) {
			// Create a new task from this function
			tasks.emplace_back({ f, &out[i] });
		};
		taskScheduler.AddTasks(funcs.size(), tasks.data(), &counter);
	}
	taskScheduler.WaitForCounter(&counter, 0);
#else
	std::transform(data.begin(), data.end() [&f](const input& in) {
		return f(in);
	}, out.begin());
#endif
	return out;
}*/
