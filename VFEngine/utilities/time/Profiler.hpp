#pragma once
#include "Clock.hpp"

namespace engineTime {
	class Profiler
	{
	private:
		inline static TimePoint startTime;

	public:
		static void start();
		static void stop();
	};
}


