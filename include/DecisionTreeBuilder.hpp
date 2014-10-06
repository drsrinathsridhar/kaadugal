#ifndef _DECISIONTREEBUILDER_HPP_
#define _DECISIONTREEBUILDER_HPP_

#include <memory>

#include "DecisionTree.hpp"
#include "AbstractDataSet.hpp"

namespace Kaadugal
{

    enum TreeBuildType
    {
	DFS,
	BFS,
	Hybrid
    };

    class TreeBuilderParameters
    {
    public:
	int m_MaxLevels;
	int m_NumCandidateThresholds;
	TreeBuildType m_Type;

	TreeBuilderParameters(int MaxLevels, int NumCandidateThresholds, TreeBuildType Type = TreeBuildType::DFS)
	    : m_MaxLevels(MaxLevels)
	    , m_NumCandidateThresholds(NumCandidateThresholds)
	    , m_Type(Type)
	{

	};

	TreeBuilderParameters(const char * ParameterFile)
	{

	};

	void Serialize()
	{

	};

	void Deserialize()
	{

	};		
    };

    // T: AbstractFeatureResponse which is the feature response function or weak learner
    // S: AbstractStatistics which contains some statistics about node from training
    // R: AbstractLeafData, arbitrary data stored if this is a leaf node
    template<class T, class S, class R>
    class DecisionTreeBuilder
    {
    private:
	std::unique_ptr<DecisionTree<T, S, R>> m_Tree;
	const AbstractDataSet& m_DataSet; // Dataset should never be modified
	const TreeBuilderParameters& m_Parameters; // Parameters also should never be modified

    public:
	DecisionTreeBuilder(const TreeBuilderParameters& Parameters)
	    : m_Parameters(Parameters)
	{
	    m_Tree = new DecisionTree<T, S, R>(Parameters.m_MaxLevels);
	};

	void BuildTree(const AbstractDataSet& DataSet)
	{
	    m_DataSet = DataSet;
	    if(m_Parameters.m_Type == TreeBuildType::DFS)
		BuildTreeDepthFirst(DataSet, 0, 0);
	    if(m_Parameters.m_Type == TreeBuildType::BFS)
		BuildTreeBreadthFirst();
	    if(m_Parameters.m_Type == TreeBuildType::Hybrid)
		BuildTreeHybrid();
	}

	void BuildTreeDepthFirst(const AbstractDataSet& PartitionedDataSet, int NodeIndex, int CurrentNodeDepth)
	{
	    if(CurrentNodeDepth > m_Tree.GetMaxDecisionLevels())
	    {
		m_Tree.GetNode(NodeIndex).MakeLeafNode();
		return;
	    }

	    // Initialize optimal values
	    VPFloat OptGain = -std::numeric_limits<VPFloat>::infinity();
	    T OptFeatureResponse; // This should create empty feature response
	    
	    for(int i = 0; i < m_Parameters.m_NumCandidateThresholds; ++i)
	    {
		T FeatureResponse(1); // This should sample from possible elements in the feature response set
	    };	    
	};

	void BuildTreeBreadthFirst(void)
	{
	    
	};

	void BuildTreeHybrid(void)
	{
	    
	};
    };
} // namespace Kaadugal

#endif // _DECISIONTREEBUILDER_HPP_
