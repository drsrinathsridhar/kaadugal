#ifndef _RANDOMIZER_HPP_
#define _RANDOMIZER_HPP_

#include <random>
#include <algorithm>

namespace Kaadugal
{

    // NOTE: This is a singleton class
    class Randomizer
    {
    public:
	static Randomizer& Get(void)
	{
	    static Randomizer R; // Instantiated on first use, guaranteed to be destroyed
	    return R;
	};

    	std::mt19937& GetRNG(void) { return m_RNG; };

    private:
	Randomizer(void) {};
	// You want to make sure they
        // are unaccessable otherwise you may accidently get copies of
        // your singleton appearing.
        Randomizer(Randomizer const&); // Don't Implement
        void operator=(Randomizer const&); // Don't implement

    	std::mt19937 m_RNG{ std::random_device{}() };
    };
} // namespace Kaadugal

#endif // _RANDOMIZER_HPP_
