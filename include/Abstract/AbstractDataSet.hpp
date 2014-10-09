#ifndef _ABSTRACTDATASET_HPP_
#define _ABSTRACTDATASET_HPP_

#include <memory>

namespace Kaadugal
{
    class AbstractDataSet
    {
    public:
	// int Size(void) = 0; // TODO: This must be virtual
	int Size(void) { return 10; };
    };
} // namespace Kaadugal

#endif // _ABSTRACTDATASET_HPP_
