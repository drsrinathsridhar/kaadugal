#pragma once

#include <stdexcept>
#include <algorithm>
#include <opencv2/highgui/highgui.hpp>

#include "Abstract/AbstractStatistics.hpp"
#include "PointSet2DRegress.hpp"

// This implements the regression from 2D data
// This class stores the leaf node stats
// These are namely: covariance of 2D position (X, Y)
// NOTE: We use cv::Mat for matrix-vector arithmetic
class GaussianStats : public Kaadugal::AbstractStatistics
{
protected:
    // NOTE: If new members are added, remember to add them to serialize/deserialize
    int m_nParams;
    Point2DRegress m_MeanParams;
    cv::Mat m_CovarianceMatrix; // This is block diagonal 2D
    int m_nDataPoints;
    
public:
    GaussianStats(void)
	: m_nDataPoints(0)
    {
	m_nParams = 2; // Default
	m_isAggregated = false;
	m_CovarianceMatrix = cv::Mat::zeros(m_nParams, m_nParams, CV_32FC1);
    };

    GaussianStats(std::shared_ptr<Kaadugal::DataSetIndex> DataSetIdx)
    {
	m_nParams = 2; // Default
	m_CovarianceMatrix = cv::Mat::zeros(m_nParams, m_nParams, CV_32FC1);

	Aggregate(DataSetIdx);
    };

    virtual ~GaussianStats(void)
    {
	m_CovarianceMatrix = cv::Mat();
    };

    virtual void Serialize(std::ostream& OutputStream) override
    {
	OutputStream.write((const char *)(&m_nParams), sizeof(int));
	OutputStream.write((const char *)(&m_nDataPoints), sizeof(int));
	OutputStream.write((const char *)(&m_MeanParams), sizeof(Point2DRegress));

	int CovMatRows = m_CovarianceMatrix.rows;
	int CovMatCols = m_CovarianceMatrix.cols;
	OutputStream.write((const char *)(&CovMatRows), sizeof(int));
	OutputStream.write((const char *)(&CovMatCols), sizeof(int));
	for(int i = 0; i < CovMatRows; ++i)
	{
	    for(int j = 0; j < CovMatCols; ++j)
	    {
		float Value = m_CovarianceMatrix.at<float>(i, j);
		OutputStream.write((const char *)(&Value), sizeof(float));
	    }
	}
    };

    virtual void Deserialize(std::istream& InputStream) override
    {
	InputStream.read((char *)(&m_nParams), sizeof(int));
	InputStream.read((char *)(&m_nDataPoints), sizeof(int));
	InputStream.read((char *)(&m_MeanParams), sizeof(Point2DRegress));

	int CovMatRows = 0;
	int CovMatCols = 0;
	InputStream.read((char *)(&CovMatRows), sizeof(int));
	InputStream.read((char *)(&CovMatCols), sizeof(int));
	m_CovarianceMatrix = cv::Mat(CovMatRows, CovMatCols, CV_32FC1);

	for(int i = 0; i < CovMatRows; ++i)
	{
	    for(int j = 0; j < CovMatCols; ++j)
	    {
		float Value = 0.0;
		InputStream.read((char *)(&Value), sizeof(float));
		m_CovarianceMatrix.at<float>(i, j) = Value;
	    }
	}
    };

    const int& GetNumParams(void) const { return m_nParams; };
    const int& GetNumDataPoints(void) const { return m_nDataPoints; };
    const cv::Mat& GetCovarianceMatrix(void) const { return m_CovarianceMatrix; };
    const Point2DRegress& GetMeanParams(void) const { return m_MeanParams; };

    virtual void Aggregate(std::shared_ptr<Kaadugal::DataSetIndex> DataSetIdx) override
    {
	std::shared_ptr<PointSet2DRegress> DerivedData = std::dynamic_pointer_cast<PointSet2DRegress>(DataSetIdx->GetDataSet());
	m_nDataPoints = DataSetIdx->Size(); // NOTE: Careful, if you take size from DerivedData, it will be wrong
	// NOTE: m_nDataPoints can be 0. This is fine. For this entropy would also be 0, as will probabilities, etc.
	if(m_nDataPoints <= 0)
	{
	    // std::cout << "[ WARN ]: Input dataset has no data points." << std::endl;
	    
	    return;
	}

	// Just compute mean and covariance, you're done!
	for(int i = 0; i < m_nDataPoints; ++i)
	{
	    auto Point2DCV = std::dynamic_pointer_cast<Point2DRegress>(DataSetIdx->GetDataPoint(i));
						       if(Point2DCV != nullptr)				
						       {
							   // std::cout << "x, y: " << Point2DCV->m_x << ", " << Point2DCV->m_y << std::endl;
	    m_MeanParams.m_x += Point2DCV->m_x;
	    m_MeanParams.m_y += Point2DCV->m_y;
						       }
							       else
								   throw std::runtime_error("Unable to get data point in Aggregate() mean computation.");

	}
	m_MeanParams.m_x /= m_nDataPoints;
	m_MeanParams.m_y /= m_nDataPoints;
	// std::cout << "x, y: " << m_MeanParams.m_x << ", " << m_MeanParams.m_y << std::endl;

	// Covariance matrix is block diagonal, Upper 3x3 block is for relative position, lower block for angles/normals
	// Upper block for positions
	for(int i = 0; i < 2; i++)
	{
	    for(int j = 0; j < 2; j++)
	    {
		m_CovarianceMatrix.at<float>(i, j) = 0.0;
		for(int k = 0; k < m_nDataPoints; k++)
		{
		    auto Point2DCV = std::dynamic_pointer_cast<Point2DRegress>(DataSetIdx->GetDataPoint(k));
							       if(Point2DCV != nullptr)
								   m_CovarianceMatrix.at<float>(i, j) += (m_MeanParams.m_x - Point2DCV->m_x) * (m_MeanParams.m_y - Point2DCV->m_y);
							       else
								   throw std::runtime_error("Unable to get data point in Aggregate()");
		}
		m_CovarianceMatrix.at<float>(i, j) /= m_nDataPoints - 1;
	    }
	}	

	// std::cout << m_MeanParams;
	// std::cout << m_CovarianceMatrix << std::endl;

	m_isAggregated = true;
    };

    virtual void Merge(std::shared_ptr<AbstractStatistics> OtherStats) override
    {
	std::shared_ptr<GaussianStats> DerivedOtherStats = std::dynamic_pointer_cast<GaussianStats>(OtherStats);
	// NOTE: It is enough to merge just the means, we don't actually care about covariances
	
	// // std::cout << DerivedOtherStats->GetNumClasses() << std::endl;
	// // std::cout << GetNumClasses() << std::endl;
	// // if(isAggregated() != true || DerivedOtherStats->isAggregated() != true)
	// //     throw std::runtime_error("Cannot merge statistics. One of them is not aggregated yet. Exiting.");

	if(DerivedOtherStats == nullptr)
	    throw std::runtime_error("Incoming statistics is null. Please check input. Exiting.");

	m_nDataPoints += DerivedOtherStats->GetNumDataPoints();
	m_MeanParams.m_x += DerivedOtherStats->GetMeanParams().m_x;
	m_MeanParams.m_y += DerivedOtherStats->GetMeanParams().m_y;
	m_MeanParams.m_x /= 2.0;
	m_MeanParams.m_y /= 2.0;

	m_isAggregated = true;
    };

    void Reset(void)
    {
	m_MeanParams = Point2DRegress();
	m_CovarianceMatrix = cv::Mat();
	
	m_nDataPoints = 0;
    };

    inline Kaadugal::VPFloat GetEntropy(void)
    {
    	if(m_isAggregated == false)
    	    return 0.0;

    	Kaadugal::VPFloat Entropy = 0.0;
	// This is equation (4) in Fanelli et al. paper not equation (3)
        // This is because we assume that the covariance is block diagonal
	// THIS IS CORRECT
	// std::cout << cv::determinant(m_CovarianceMatrix(cv::Rect(0,0,3,3))) << std::endl;
	// std::cout << m_CovarianceMatrix(cv::Rect(0,0,3,3)) << std::endl;
	// std::cout << cv::determinant(m_CovarianceMatrix(cv::Rect(3,3,3,3))) << std::endl;
	// std::cout << m_CovarianceMatrix(cv::Rect(3,3,3,3)) << std::endl;
	Entropy = log( cv::determinant(m_CovarianceMatrix) ) / log(2.0); // Using base-2 log

	// std::cout << "Matrix:\n " << m_CovarianceMatrix << std::endl;
	// std::cout << "Determinant: " << cv::determinant(m_CovarianceMatrix) << std::endl;
	// std::cout << "Entropy: " << Entropy << std::endl;

	// THIS IS WRONG
	// Entropy = log(cv::determinant(m_CovarianceMatrix)) / log(2.0); // Using base-2 log for entropy

    	return Entropy;
    };
};
