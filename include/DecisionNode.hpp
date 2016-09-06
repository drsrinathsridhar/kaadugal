#pragma once

#include <limits>
#include <cmath>
#include <fstream>

namespace Kaadugal
{
	// Use Variable Precision Floating point representation
	typedef float VPFloat; // Change to float or long double as needed
	enum NodeType
	{
		SplitNode,
		LeafNode,
		Invalid
	};

	// T: AbstractFeatureResponse which is the feature response function or weak learner
	// S: AbstractStatistics which contains some statistics about node from training
	// R: AbstractLeafData, arbitrary data stored if this is a leaf node
	template<class T, class S, class R>
	class DecisionNode
	{
	private:
		Kaadugal::NodeType m_Type;

		// Even leaf nodes store a threshold if needed. Might be useful for growing trees after training
		VPFloat m_Threshold; // The decision threshold
		T m_FeatureResponse; // The feature response function
		S m_Statistics; // Node statistics
		R m_Data; // Arbitrary leaf data

	public:
		DecisionNode(void) // With no arguments, we construct an invalid node
			: m_Type(Kaadugal::Invalid)
			, m_Threshold(std::numeric_limits<VPFloat>::quiet_NaN())
			, m_FeatureResponse(T())
			, m_Statistics(S())
			, m_Data(R())
		{

		};

		void operator=(const DecisionNode<T, S, R>& RHS)
		{
			m_Threshold = RHS.m_Threshold;
			m_FeatureResponse = RHS.m_FeatureResponse;
			m_Statistics = RHS.m_Statistics;
			m_Data = RHS.m_Data;
			m_Type = RHS.m_Type;
		};

		void Serialize(std::ostream& OutputStream) const
		{
			OutputStream.write((const char *)(&m_Type), sizeof(Kaadugal::NodeType));
			OutputStream.write((const char *)(&m_Threshold), sizeof(VPFloat));
			m_FeatureResponse.Serialize(OutputStream);
			m_Statistics.Serialize(OutputStream);
			m_Data.Serialize(OutputStream);
		};

		void Deserialize(std::istream& InputStream)
		{
			InputStream.read((char *)(&m_Type), sizeof(Kaadugal::NodeType));
			InputStream.read((char *)(&m_Threshold), sizeof(VPFloat));
			m_FeatureResponse.Deserialize(InputStream);
			m_Statistics.Deserialize(InputStream);
			m_Data.Deserialize(InputStream);
		};

		void MakeSplitNode(S Statistics, T FeatureResponse, VPFloat Threshold)
		{
			m_Type = Kaadugal::SplitNode;
			m_Threshold = Threshold;
			m_FeatureResponse = FeatureResponse; // Deep copy. TODO: Do we need deep copy?
			m_Statistics = Statistics; // Deep copy	    
		};

		void MakeLeafNode(S Statistics, R Data = R())
		{
			// NOTE: If not leaf data is passed, then m_Data is also dummy. This is fine
			m_Type = Kaadugal::LeafNode;
			m_Threshold = std::numeric_limits<VPFloat>::quiet_NaN(); // Leaves don't have thresholds
			m_FeatureResponse = T(); // TODO: Leaves don't need feature responses. So dummy
			m_Statistics = Statistics; // Deep copy
			m_Data = Data; // Deep copy
		};

		const R& GetLeafData(void) const { return m_Data; };
		const S& GetStatistics(void) const { return m_Statistics; };
		T& GetFeatureResponse(void) { return m_FeatureResponse; };
		const VPFloat& GetThreshold(void) const { return m_Threshold; };
		Kaadugal::NodeType GetType(void) const { return m_Type; };

		// Render methods for visualizing node
		virtual void Render(void)
		{
			// TODO:
		};
	};
} // namespace Kaadugal
