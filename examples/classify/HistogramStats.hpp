#ifndef _HISTOGRAMSTATS_HPP_
#define _HISTOGRAMSTATS_HPP_

#include "Abstract/AbstractStatistics.hpp"

class HistogramStats : public Kaadugal::AbstractStatistics
{
public:
    HistogramStats(void)
    {
	m_isAggregated = false;
    };

    HistogramStats(std::shared_ptr<Kaadugal::DataSetIndex> DataSetIdx)
    {
	Aggregate(DataSetIdx);
    };

    virtual void Aggregate(std::shared_ptr<Kaadugal::DataSetIndex> DataSetIdx) override
    {
	m_isAggregated = true;
    };    
};

#endif // _HISTOGRAMSTATS_HPP_
