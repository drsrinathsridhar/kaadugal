#ifndef _HISTOGRAMSTATS_HPP_
#define _HISTOGRAMSTATS_HPP_

#include <stdexcept>
#include <algorithm>

#include "Abstract/AbstractStatistics.hpp"

// Supports histogram-like statistics for arbitrary number of classes
class HistogramStats : public Kaadugal::AbstractStatistics
{
protected:
    // NOTE: If new members are added, remember to add them to serialize/deserialize
    int m_nClasses; // This is also the number of bins
    std::vector<int> m_Bins;
    int m_nDataPoints;
    
public:
    HistogramStats(void)
	: m_nClasses(0)
	, m_nDataPoints(0)
    {
	m_isAggregated = false;
    };

    HistogramStats(const int nClasses)
	: m_nClasses(nClasses)
	, m_nDataPoints(0)
    {
	m_Bins.resize(m_nClasses, 0);
    };

    HistogramStats(std::shared_ptr<Kaadugal::DataSetIndex> DataSetIdx)
    {
	Aggregate(DataSetIdx);
    };

    virtual void Serialize(std::ostream& OutputStream) override
    {
	OutputStream.write((const char *)(&m_nClasses), sizeof(int));
	OutputStream.write((const char *)(&m_nDataPoints), sizeof(int));

	int BinVecSize = m_Bins.size();
	OutputStream.write((const char *)(&BinVecSize), sizeof(int));
	for(int i = 0; i < BinVecSize; ++i)
	    OutputStream.write((const char *)(&m_Bins[i]), sizeof(int));
    };

    virtual void Deserialize(std::istream& InputStream) override
    {
	InputStream.read((char *)(&m_nClasses), sizeof(int));
	InputStream.read((char *)(&m_nDataPoints), sizeof(int));

	int BinVecSize = 0;
	InputStream.read((char *)(&BinVecSize), sizeof(int));
	m_Bins.resize(BinVecSize, 0);
	for(int i = 0; i < BinVecSize; ++i)
	    InputStream.read((char *)(&m_Bins[i]), sizeof(int));
    };

    const int& GetNumClasses(void) const { return m_nClasses; };
    const int& GetNumDataPoints(void) const { return m_nDataPoints; };
    const std::vector<int>& GetBins(void) const { return m_Bins; };

    virtual void Aggregate(std::shared_ptr<Kaadugal::DataSetIndex> DataSetIdx) override
    {
	std::shared_ptr<PointSet2D> DerivedData = std::dynamic_pointer_cast<PointSet2D>(DataSetIdx->GetDataSet());
	m_nClasses = DerivedData->GetNumClasses();
	m_Bins.clear(); // TODO: Is this necessary and, if so, is it efficient?
	m_Bins.resize(m_nClasses, 0);

	m_nDataPoints = DataSetIdx->Size(); // NOTE: Careful, if you take size from DerivedData, it will be wrong
	// NOTE: m_nDataPoints can be 0. This is fine. For this entropy would also be 0, as will probabilities, etc.
	// if(m_nDataPoints <= 0)
	// {
	//     std::cout << "[ WARN ]: Input dataset has no data points." << std::endl;
	//     return;
	// }

	for(int i = 0; i < m_nDataPoints; ++i)
	{
	    int DataLabel = std::dynamic_pointer_cast<Point2D>(DataSetIdx->GetDataPoint(i))->GetLabel();
	    if(DataLabel > m_nClasses-1)
		throw std::runtime_error("Data point label is inconsistent with number of classes. Exiting.");

	    m_Bins[DataLabel]++;
	}	

	m_isAggregated = true;
    };

    virtual void Merge(std::shared_ptr<AbstractStatistics> OtherStats) override
    {
	std::shared_ptr<HistogramStats> DerivedOtherStats = std::dynamic_pointer_cast<HistogramStats>(OtherStats);
	if(DerivedOtherStats->GetNumClasses() != GetNumClasses())
	    throw std::runtime_error("Cannot merge statistics. Number of classes don't match. Exiting.");

	// std::cout << DerivedOtherStats->GetNumClasses() << std::endl;
	// std::cout << GetNumClasses() << std::endl;
	// if(isAggregated() != true || DerivedOtherStats->isAggregated() != true)
	//     throw std::runtime_error("Cannot merge statistics. One of them is not aggregated yet. Exiting.");

	if(DerivedOtherStats == nullptr)
	    throw std::runtime_error("Incoming statistics is null. Please check input. Exiting.");

	m_nDataPoints += DerivedOtherStats->GetNumDataPoints();
	for(int i = 0; i < m_nClasses; ++i)
	    m_Bins[i] += DerivedOtherStats->GetBins()[i];

	m_isAggregated = true;
    };

    void Reset(void) // Reset bin counts to 0
    {
	m_Bins.clear();
	
	m_nDataPoints = 0;
    };

    inline Kaadugal::VPFloat GetProbability(int ClassLabel) const
    {
	if(m_nDataPoints <= 0)
	    return 0.0;

	return Kaadugal::VPFloat(m_Bins[ClassLabel]) / Kaadugal::VPFloat(m_nDataPoints);
    };

    int FindWinnerLabelIndex() const
    {
	return std::distance(m_Bins.begin(), std::max_element(m_Bins.begin(), m_Bins.end())); // Return index of class label with most data points
    }

    Kaadugal::VPFloat GetEntropy(void)
    {
	if(m_isAggregated == false)
	    return 0.0;

	Kaadugal::VPFloat Entropy = 0.0;
	for(int i = 0; i < m_nClasses; ++i)
	{
	    Kaadugal::VPFloat p = GetProbability(i);
	    Entropy -= p == 0.0 ? 0.0 : (p * log(p) ) / log(2.0);
	}

	return Entropy;
    };
};

#endif // _HISTOGRAMSTATS_HPP_
