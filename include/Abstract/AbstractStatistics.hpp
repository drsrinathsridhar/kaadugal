#ifndef _ABSTRACTSTATISTICS_HPP_
#define _ABSTRACTSTATISTICS_HPP_

#include <memory>
#include <ostream>
#include <istream>

#include "DataSetIndex.hpp"

namespace Kaadugal
{
    class AbstractStatistics
    {
    public:
	AbstractStatistics(void)
	{
	    m_isAggregated = false;
	    m_isValid = false;
	};
	virtual void Serialize(std::ostream& OutputStream) = 0;
	virtual void Deserialize(std::istream& InputStream) = 0;

	virtual void Aggregate(std::shared_ptr<DataSetIndex> DataSetIdx) = 0;
	virtual void Merge(std::shared_ptr<AbstractStatistics> OtherStats) = 0; // Merging two stats together
	virtual bool isAggregated(void) { return m_isAggregated; };
	virtual bool isValid(void) { return m_isValid; };

    protected:
	bool m_isAggregated;	
	bool m_isValid;
    };
} // namespace Kaadugal

#endif // _ABSTRACTSTATISTICS_HPP_

