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
	    : m_isBuilt(false)
	{
	
	};
	AbstractStatistics(std::shared_ptr<DataSetIndex> DataSetIdx)
	{
	    Build(DataSetIdx);
	};

	void Build(std::shared_ptr<DataSetIndex> DataSetIdx)
	{
	    m_isBuilt = true;
	};

    private:
	bool m_isBuilt;
	
    };
} // namespace Kaadugal

#endif // _ABSTRACTSTATISTICS_HPP_

