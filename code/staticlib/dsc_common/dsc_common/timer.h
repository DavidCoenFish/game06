#pragma once
#include "dsc_common.h"

/*
	alt names: stopwatch
	return seconds since last call to GetDeltaSeconds, or time since class ctor on first call

	todo: remove all the [_delta_seconds_average, _nice_fps] extra stuff
*/
namespace DscCommon
{
	class Timer
	{
	public:
		Timer();

		const float GetDeltaSeconds(float* const out_nice_fps_or_nullptr = nullptr);

	private:
		std::chrono::steady_clock::time_point _time_point;

		float _delta_seconds_average;
		float _nice_fps;
		float _nice_fps_time_accumulate;

	}; //class Timer
} // namespace DscCommon