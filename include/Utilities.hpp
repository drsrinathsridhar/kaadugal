#ifndef _UTILITIES_HPP_
#define _UTILITIES_HPP_

// Get the current Epoch time in microseconds
inline uint64_t GetCurrentEpochTime(void)
{
    return uint64_t(std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1)); // This is C++11. Epoch time in microseconds
};

#endif // #ifndef _UTILITIES_HPP_
