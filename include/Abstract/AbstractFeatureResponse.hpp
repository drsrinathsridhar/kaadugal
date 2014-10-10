#ifndef _ABSTRACTFEATURERESPONSE_HPP_
#define _ABSTRACTFEATURERESPONSE_HPP_

#include <memory>

#include "AbstractDataSet.hpp"

namespace Kaadugal
{
    class AbstractFeatureResponse
    {
    public:
	virtual VPFloat GetResponse(std::shared_ptr<AbstractDataPoint> DataPoint) = 0;
    };
} // namespace Kaadugal

#endif // _ABSTRACTFEATURERESPONSE_HPP_
