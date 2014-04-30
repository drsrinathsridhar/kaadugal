#ifndef _DECISIONNODE_HPP_
#define _DECISIONNODE_HPP_

#include <limits>
#include <cmath>
#include <fstream>

namespace Kaadugal
{
    // Use Variable Prevision Floating point representation
    typedef double VPFloat; // or float or long double
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
	// Even leaf nodes store a threshold if needed. Might be useful for growing trees after training
	VPFloat m_Threshold; // The decision threshold
	T m_FeatureResponse; // The feature response function
	S m_Statistics; // Node statistics
	R m_Data; // Arbitrary leaf data

	Kaadugal::NodeType m_Type;

    public:
	DecisionNode(void) // With no arguments, we construct an invalid node
	    : m_Threshold(std::numeric_limits<VPFloat>::quiet_NaN())
	    , m_FeatureResponse(T())
	    , m_Statistics(S())
	    , m_Data(R())
	    , m_Type(Kaadugal::Invalid)
	{
	    
	};

	DecisionNode<T, S, R>& operator=(const DecisionNode<T, S, R>& RHS)
	{
	    m_Threshold = RHS.m_Threshold;
	    m_FeatureResponse = RHS.m_FeatureResponse;
	    m_Statistics = RHS.m_Statistics;
	    m_Data = RHS.m_Data;
	    m_Type = RHS.m_Type;
	};

	void MakeSplitNode(T FeatureResponse, S Statistics, VPFloat Threshold)
	{
	    m_Type = Kaadugal::SplitNode;
	    m_Threshold = Threshold;
	    m_FeatureResponse = FeatureResponse; // Deep copy. TODO: Do we need deep copy?
	    m_Statistics = Statistics; // Deep copy	    
	};

	void MakeLeafNode(T FeatureResponse, S Statistics, R Data)
	{
	    m_Type = Kaadugal::LeafNode;
	    m_Threshold = std::numeric_limits<VPFloat>::quiet_NaN(); // Leaves don't have thresholds
	    m_FeatureResponse = FeatureResponse; // Deep copy. TODO: Do we need deep copy?
	    m_Statistics = Statistics; // Deep copy
	    m_Data = Data; // Deep copy
	};

	const R& GetLeafData(void) { return m_Data; };
	const S& GetStatistics(void) { return m_Statistics; };
	const Kaadugal::NodeType GetType(void) { return m_Type; };

	// Stream write methods
	virtual void Serialize(std::ostream& Out) const
	{
	    // TODO:
	};
	virtual void Deserialize(std::istream& In)
	{
	    // TODO:
	};

	// Render methods for visualizing node
	virtual void Render(void)
	{
	    // TODO:
	};
    };
} // namespace Kaadugal

#endif // _DECISIONNODE_HPP_
