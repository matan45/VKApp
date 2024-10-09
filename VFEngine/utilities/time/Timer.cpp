#include "Timer.hpp"
namespace engineTime {
	void Timer::initialize()
	{
		startTime = Clock::now();
		lastTime = startTime;
		currentTime = startTime;
		deltaTime = 0.0;
	}

	void Timer::update()
	{
		currentTime = Clock::now();
		Duration frameTime = std::chrono::duration_cast<Duration>(currentTime - lastTime);
		deltaTime = frameTime.count();  // Get deltaTime in seconds
		lastTime = currentTime;
	}

	double Timer::getDeltaTime()
	{
		return deltaTime;
	}

	double Timer::getElapsedTime()
	{
		Duration elapsed = std::chrono::duration_cast<Duration>(currentTime - startTime);
		return elapsed.count();  // Return time in seconds
	}

	double Timer::getFPS()
	{
		if (deltaTime > 0) {
			return 1.0 / deltaTime;
		}
		return 0.0;
	}

	void Timer::reset()
	{
		startTime = Clock::now();
		lastTime = startTime;
		currentTime = startTime;
	}

}
