#ifndef _UTILITIES_HPP_
#define _UTILITIES_HPP_

#include <chrono>

// Get the current Epoch time in microseconds
inline uint64_t GetCurrentEpochTime(void)
{
	uint64_t etime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	return etime;
};

#endif // #ifndef _UTILITIES_HPP_
