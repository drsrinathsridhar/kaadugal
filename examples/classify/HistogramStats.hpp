#ifndef _HISTOGRAMSTATS_HPP_
#define _HISTOGRAMSTATS_HPP_

#include <stdexcept>
#include <algorithm>

#include "Abstract/AbstractStatistics.hpp"

// Supports histogram-like statistics for arbitrary number of classes
class HistogramStats : public Kaadugal::AbstractStatistics
{
protected:
    int m_nClasses; // This is also the number of bins
    std::vector<int> m_Bins;
    int m_nDataPoints;
    
public:
    HistogramStats(void)
    {
	m_isAggregated = false;
    };

    HistogramStats(std::shared_ptr<Kaadugal::DataSetIndex> DataSetIdx)
    {
	Aggregate(DataSetIdx);
    };

    const int GetNumClasses(void) const { return m_nClasses; };
    const int GetNumDataPoints(void) const { return m_nDataPoints; };
    const std::vector<int>& GetBins(void) const { return m_Bins; };

    virtual void Aggregate(std::shared_ptr<Kaadugal::DataSetIndex> DataSetIdx) override
    {
	std::shared_ptr<PointSet2D> DerivedDataIdx = std::dynamic_pointer_cast<PointSet2D>(DataSetIdx->GetDataSet());
	m_nClasses = DerivedDataIdx->GetNumClasses();
	m_Bins.clear(); // TODO: Is this necessary and, if so, is it efficient?
	m_Bins.resize(m_nClasses, 0);

	m_nDataPoints = DerivedDataIdx->Size();
	if(m_nDataPoints <= 0)
	    return;

	for(int i = 0; i < m_nDataPoints; ++i)
	{
	    int DataLabel = std::dynamic_pointer_cast<Point2D>(DerivedDataIdx->Get(i))->GetLabel();
	    if(DataLabel > m_nClasses-1)
		throw std::runtime_error("Data point label is inconsistent with number of classes. Exiting.");

	    m_Bins[DataLabel]++;
	}	

	m_isAggregated = true;
    };

    virtual void Merge(std::shared_ptr<AbstractStatistics> OtherStats)
    {
	std::shared_ptr<HistogramStats> DerivedOtherStats = std::dynamic_pointer_cast<HistogramStats>(OtherStats);
	if(DerivedOtherStats->GetNumClasses() != GetNumClasses())
	    throw std::runtime_error("Cannot merge statistics. Number of classes don't match. Exiting.");

	if(isAggregated() != true || DerivedOtherStats->isAggregated())
	    throw std::runtime_error("Cannot merge statistics. One of them is not aggregated yet. Exiting.");

	m_nDataPoints += DerivedOtherStats->GetNumDataPoints();
	for(int i = 0; i < m_nClasses; ++i)
	    m_Bins[i] += DerivedOtherStats->GetBins()[i];

	m_isAggregated = true;
    };

    Kaadugal::VPFloat GetProbability(int ClassLabel) const
    {
	// TODO: Verify
	return Kaadugal::VPFloat(m_Bins[ClassLabel]) / Kaadugal::VPFloat(m_nDataPoints);
    };

    int FindWinnerLabelIndex() const
    {
	// TODO: Verify
	return std::distance(m_Bins.begin(), std::max_element(m_Bins.begin(), m_Bins.end())); // Return index of class label with most data points
    }

    Kaadugal::VPFloat GetEntropy(void)
    {
	if(m_isAggregated == false)
	    return 0.0;

	Kaadugal::VPFloat Entropy = 0.0;
	for(int i = 0; i < m_nClasses; ++i)
	{
	    Kaadugal::VPFloat p = Kaadugal::VPFloat(m_Bins[i]) / Kaadugal::VPFloat(m_nDataPoints);
	    Entropy -= p == 0.0 ? 0.0 : (p * log(p) ) / log(2.0);
	}

	return Entropy;
    };
};

#endif // _HISTOGRAMSTATS_HPP_
