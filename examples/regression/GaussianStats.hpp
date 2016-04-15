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
class GaussianStats
	: public Kaadugal::AbstractStatistics
{
protected:
	// NOTE: If new members are added, remember to add them to serialize/deserialize
	int m_nParams;
	cv::Mat m_MeanParams;
	cv::Mat m_CovarianceMatrix; // This is block diagonal 2D
	int m_nDataPoints;

public:
	GaussianStats(void)
		: m_nDataPoints(0)
	{
		m_nParams = 1; // Default
		m_isAggregated = false;
		m_MeanParams = cv::Mat::zeros(m_nParams, 1, CV_32FC1);
		m_CovarianceMatrix = cv::Mat::zeros(m_nParams, m_nParams, CV_32FC1);
	};

	GaussianStats(std::shared_ptr<Kaadugal::DataSetIndex> DataSetIdx)
	{
		m_nParams = 1; // Default
		m_MeanParams = cv::Mat::zeros(m_nParams, 1, CV_32FC1);
		m_CovarianceMatrix = cv::Mat::zeros(m_nParams, m_nParams, CV_32FC1);

		Aggregate(DataSetIdx);
	};

	virtual ~GaussianStats(void)
	{
		m_CovarianceMatrix = cv::Mat();
	};

	virtual void Serialize(std::ostream& OutputStream) const override
	{
		OutputStream.write((const char *)(&m_nParams), sizeof(int));
		OutputStream.write((const char *)(&m_nDataPoints), sizeof(int));

		int MeanMatRows = m_MeanParams.rows; // Same as nParams
		OutputStream.write((const char *)(&MeanMatRows), sizeof(int));
		for (int i = 0; i < MeanMatRows; ++i)
		{
			float Value = m_MeanParams.at<float>(i, 0);
			OutputStream.write((const char *)(&Value), sizeof(float));
		}

		int CovMatRows = m_CovarianceMatrix.rows;
		int CovMatCols = m_CovarianceMatrix.cols;
		OutputStream.write((const char *)(&CovMatRows), sizeof(int));
		OutputStream.write((const char *)(&CovMatCols), sizeof(int));
		for (int i = 0; i < CovMatRows; ++i)
		{
			for (int j = 0; j < CovMatCols; ++j)
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

		int MeanMatRows = 0;
		InputStream.read((char *)(&MeanMatRows), sizeof(int));
		m_MeanParams = cv::Mat(MeanMatRows, 1, CV_32FC1);

		for (int i = 0; i < MeanMatRows; ++i)
		{
			float Value = 0.0;
			InputStream.read((char *)(&Value), sizeof(float));
			m_MeanParams.at<float>(i, 0) = Value;
		}

		int CovMatRows = 0;
		int CovMatCols = 0;
		InputStream.read((char *)(&CovMatRows), sizeof(int));
		InputStream.read((char *)(&CovMatCols), sizeof(int));
		m_CovarianceMatrix = cv::Mat(CovMatRows, CovMatCols, CV_32FC1);

		for (int i = 0; i < CovMatRows; ++i)
		{
			for (int j = 0; j < CovMatCols; ++j)
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
	const cv::Mat& GetMeanParams(void) const { return m_MeanParams; };

	virtual void Aggregate(std::shared_ptr<Kaadugal::DataSetIndex> DataSetIdx) override
	{
		std::shared_ptr<PointSet2DRegress> DerivedData = std::dynamic_pointer_cast<PointSet2DRegress>(DataSetIdx->GetDataSet());
		m_nDataPoints = DataSetIdx->Size(); // NOTE: Careful, if you take size from DerivedData, it will be wrong
		// NOTE: m_nDataPoints can be 0. This is fine. For this entropy would also be 0, as will probabilities, etc.
		if (m_nDataPoints <= 0)
		{
			// std::cout << "[ WARN ]: Input dataset has no data points." << std::endl;

			return;
		}

		// Just compute mean and covariance, you're done!
		m_MeanParams.setTo(0.0);
		for (int i = 0; i < m_nDataPoints; ++i)
		{
			auto Point2DCV = std::dynamic_pointer_cast<Point2DRegress>(DataSetIdx->GetDataPoint(i));
			if (Point2DCV != nullptr)
				m_MeanParams.at<float>(0, 0) += Point2DCV->m_Value;
			else
				throw std::runtime_error("Unable to get data point in Aggregate() mean computation.");

		}
		m_MeanParams.at<float>(0, 0) /= m_nDataPoints;

		// Covariance matrix
		for (int i = 0; i < m_nParams; i++)
		{
			for (int j = 0; j < m_nParams; j++)
			{
				m_CovarianceMatrix.at<float>(i, j) = 0.0;
				for (int k = 0; k < m_nDataPoints; k++)
				{
					auto Point2DCV = std::dynamic_pointer_cast<Point2DRegress>(DataSetIdx->GetDataPoint(k));
					if (Point2DCV != nullptr)
					{
						float Diff1 = m_MeanParams.at<float>(i, 0) - (*Point2DCV).m_Value;
						float Diff2 = m_MeanParams.at<float>(j, 0) - (*Point2DCV).m_Value;
						m_CovarianceMatrix.at<float>(i, j) += Diff1 * Diff2;
					}
					else
						throw std::runtime_error("Unable to get data point in Aggregate()");
				}
				m_CovarianceMatrix.at<float>(i, j) /= float(m_nDataPoints - 1);
			}
		}

		//std::cout << m_MeanParams;
		//std::cout << m_CovarianceMatrix << std::endl;

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

		if (DerivedOtherStats == nullptr)
			throw std::runtime_error("Incoming statistics is null. Please check input. Exiting.");

		auto IncNumDataPoints = DerivedOtherStats->GetNumDataPoints();
		auto CurrNumDataPoints = m_nDataPoints;
		m_MeanParams.at<float>(0, 0) = m_MeanParams.at<float>(0, 0) * float(CurrNumDataPoints) + DerivedOtherStats->GetMeanParams().at<float>(0, 0) * float(IncNumDataPoints);
		m_nDataPoints += IncNumDataPoints;

		m_MeanParams.at<float>(0, 0) /= float(m_nDataPoints);


		m_isAggregated = true;
	};

	void Reset(void)
	{
		m_MeanParams = cv::Mat();
		m_CovarianceMatrix = cv::Mat();

		m_nDataPoints = 0;
	};

	inline Kaadugal::VPFloat GetEntropy(void)
	{
		if (m_isAggregated == false)
			return 0.0;

		Kaadugal::VPFloat Entropy = 0.0;
		//std::cout << m_CovarianceMatrix << std::endl;
		//std::cout << cv::determinant(m_CovarianceMatrix) << std::endl;
		Entropy = log(cv::determinant(m_CovarianceMatrix)) / log(2.0); // Using base-2 log

		// std::cout << "Matrix:\n " << m_CovarianceMatrix << std::endl;
		// std::cout << "Determinant: " << cv::determinant(m_CovarianceMatrix) << std::endl;
		// std::cout << "Entropy: " << Entropy << std::endl;

		// THIS IS WRONG
		// Entropy = log(cv::determinant(m_CovarianceMatrix)) / log(2.0); // Using base-2 log for entropy

		return Entropy;
	};
};
