#ifndef _STRUCTDATASTATS_HPP_
#define _STRUCTDATASTATS_HPP_

#include <stdexcept>
#include <algorithm>

#include "Abstract/AbstractStatistics.hpp"

// Supports histogram-like statistics for structured output, each dimension of which may have arbitrary number of classes
class StructDataStats : public Kaadugal::AbstractStatistics
{
protected:
    // NOTE: If new members are added, remember to add them to serialize/deserialize
    int m_nClassesPerDim; // Number of classes per dimension of the structured label
    std::vector<int> m_DimClassBins;
    int m_nDataPoints;
    
public:
    StructDataStats(void)
	: m_nClassesPerDim(0)
	, m_nDataPoints(0)
    {
	m_isAggregated = false;
    };

    StructDataStats(const int nClasses)
	: m_nClassesPerDim(nClasses)
	, m_nDataPoints(0)
    {
	m_DimClassBins.resize(m_nClassesPerDim, 0);
    };

    StructDataStats(std::shared_ptr<Kaadugal::DataSetIndex> DataSetIdx)
    {
	Aggregate(DataSetIdx);
    };

    virtual void Serialize(std::ostream& OutputStream) override
    {
	OutputStream.write((const char *)(&m_nClassesPerDim), sizeof(int));
	OutputStream.write((const char *)(&m_nDataPoints), sizeof(int));

	int BinVecSize = m_DimClassBins.size();
	OutputStream.write((const char *)(&BinVecSize), sizeof(int));
	for(int i = 0; i < BinVecSize; ++i)
	    OutputStream.write((const char *)(&m_DimClassBins[i]), sizeof(int));
    };

    virtual void Deserialize(std::istream& InputStream) override
    {
	InputStream.read((char *)(&m_nClassesPerDim), sizeof(int));
	InputStream.read((char *)(&m_nDataPoints), sizeof(int));

	int BinVecSize = 0;
	InputStream.read((char *)(&BinVecSize), sizeof(int));
	m_DimClassBins.resize(BinVecSize, 0);
	for(int i = 0; i < BinVecSize; ++i)
	    InputStream.read((char *)(&m_DimClassBins[i]), sizeof(int));
    };

    const int& GetNumClasses(void) const { return m_nClassesPerDim; };
    const int& GetNumDataPoints(void) const { return m_nDataPoints; };
    const std::vector<int>& GetBins(void) const { return m_DimClassBins; };

    virtual void Aggregate(std::shared_ptr<Kaadugal::DataSetIndex> DataSetIdx) override
    {
	std::shared_ptr<PointSet2D> DerivedData = std::dynamic_pointer_cast<PointSet2D>(DataSetIdx->GetDataSet());
	m_nClassesPerDim = DerivedData->GetNumClasses();
	m_DimClassBins.clear(); // TODO: Is this necessary and, if so, is it efficient?
	m_DimClassBins.resize(m_nClassesPerDim, 0);

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
	    if(DataLabel > m_nClassesPerDim-1)
		throw std::runtime_error("Data point label is inconsistent with number of classes. Exiting.");

	    m_DimClassBins[DataLabel]++;
	}	

	m_isAggregated = true;
    };

    virtual void Merge(std::shared_ptr<AbstractStatistics> OtherStats) override
    {
	std::shared_ptr<StructDataStats> DerivedOtherStats = std::dynamic_pointer_cast<StructDataStats>(OtherStats);
	if(DerivedOtherStats->GetNumClasses() != GetNumClasses())
	    throw std::runtime_error("Cannot merge statistics. Number of classes don't match. Exiting.");

	// std::cout << DerivedOtherStats->GetNumClasses() << std::endl;
	// std::cout << GetNumClasses() << std::endl;
	// if(isAggregated() != true || DerivedOtherStats->isAggregated() != true)
	//     throw std::runtime_error("Cannot merge statistics. One of them is not aggregated yet. Exiting.");

	if(DerivedOtherStats == nullptr)
	    throw std::runtime_error("Incoming statistics is null. Please check input. Exiting.");

	m_nDataPoints += DerivedOtherStats->GetNumDataPoints();
	for(int i = 0; i < m_nClassesPerDim; ++i)
	    m_DimClassBins[i] += DerivedOtherStats->GetBins()[i];

	m_isAggregated = true;
    };

    void Reset(void) // Reset bin counts to 0
    {
	m_DimClassBins.clear();
	
	m_nDataPoints = 0;
    };

    inline Kaadugal::VPFloat GetProbability(int ClassLabel) const
    {
	if(m_nDataPoints <= 0)
	    return 0.0;

	return Kaadugal::VPFloat(m_DimClassBins[ClassLabel]) / Kaadugal::VPFloat(m_nDataPoints);
    };

    int FindWinnerLabelIndex() const
    {
	return std::distance(m_DimClassBins.begin(), std::max_element(m_DimClassBins.begin(), m_DimClassBins.end())); // Return index of class label with most data points
    }

    Kaadugal::VPFloat GetEntropy(void)
    {
	if(m_isAggregated == false)
	    return 0.0;

	Kaadugal::VPFloat Entropy = 0.0;
	for(int i = 0; i < m_nClassesPerDim; ++i)
	{
	    Kaadugal::VPFloat p = GetProbability(i);
	    Entropy -= p == 0.0 ? 0.0 : (p * log(p) ) / log(2.0);
	}

	return Entropy;
    };
};

#endif // _STRUCTDATASTATS_HPP_
