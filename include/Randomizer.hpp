#ifndef _RANDOMIZER_HPP_
#define _RANDOMIZER_HPP_

#include <random>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <omp.h>

namespace Kaadugal
{
	// This is a singleton class but supports multiple *OpenMP* threads
	// Note, however, that no other threading library is supported
	// See also http://stackoverflow.com/questions/15918758/how-to-make-each-thread-use-its-own-rng-in-c11
	class Randomizer
	{
	private:
		Randomizer(void)
		{
			int nThreads = std::max(1, omp_get_max_threads());
			std::cout << "[ INFO ]: Maximum usable threads: " << nThreads << std::endl;
			for (int i = 0; i < nThreads; ++i)
			{
				std::random_device SeedDevice;
				m_RandEngines.push_back(std::mt19937(SeedDevice()));
				//m_RandEngines.push_back(std::mt19937((unsigned)0));
			}
		};
		// You want to make sure they
		// are unaccessable otherwise you may accidently get copies of
		// your singleton appearing.
		Randomizer(Randomizer const&); // Don't Implement
		void operator=(Randomizer const&); // Don't implement

		std::vector<std::mt19937> m_RandEngines;

	public:
		static Randomizer& Get(void)
		{
			static Randomizer R; // Instantiated on first use, guaranteed to be destroyed
			return R;
		};

		// Choose random element from an STL vector
		template <typename T>
		static T GetRandomElement(const std::vector<T>& Vec)
		{
			std::uniform_int_distribution<> dis(0, Vec.size() - 1);
			return Vec[dis(Randomizer::Get().GetRNG())];
		};

		std::mt19937& GetRNG(void)
		{
			int ThreadID = omp_get_thread_num();
			// std::cout << "Thread ID: " << ThreadID << std::endl;
			return m_RandEngines[ThreadID];
		};
	};
} // namespace Kaadugal

#endif // _RANDOMIZER_HPP_

