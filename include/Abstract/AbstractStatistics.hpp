#ifndef _ABSTRACTSTATISTICS_HPP_
#define _ABSTRACTSTATISTICS_HPP_

#include <memory>

#include "DataSetIndex.hpp"

namespace Kaadugal
{
    class AbstractStatistics
    {
    public:
	AbstractStatistics(void)
	{
	    m_isAggregated = false;
	};
	AbstractStatistics(std::shared_ptr<DataSetIndex> DataSetIdx)
	{
	    Aggregate(DataSetIdx);
	};

	virtual void Aggregate(std::shared_ptr<DataSetIndex> DataSetIdx) = 0;

    protected:
	bool m_isAggregated;	
    };
} // namespace Kaadugal

#endif // _ABSTRACTSTATISTICS_HPP_

