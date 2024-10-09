#include "Profiler.hpp"

namespace engineTime {
	void Profiler::start()
	{
		startTime = Clock::now();
	}

	void Profiler::stop()
	{
		TimePoint endTime = Clock::now();
		Duration timeTaken = std::chrono::duration_cast<Duration>(endTime - startTime);
		//std::cout << "Time taken: " << timeTaken.count() << " seconds" << std::endl;
	}

}
