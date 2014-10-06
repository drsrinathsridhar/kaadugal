#ifndef _DECISIONTREEBUILDER_HPP_
#define _DECISIONTREEBUILDER_HPP_

#include <memory>

#include "DecisionTree.hpp"
#include "AbstractDataSet.hpp"
#include "Parameters.hpp"

namespace Kaadugal
{
    // T: AbstractFeatureResponse which is the feature response function or weak learner
    // S: AbstractStatistics which contains some statistics about node from training
    // R: AbstractLeafData, arbitrary data stored if this is a leaf node
    template<class T, class S, class R>
    class DecisionTreeBuilder
    {
    private:
	std::shared_ptr<DecisionTree<T, S, R>> m_Tree;
	AbstractDataSet m_DataSet; // Dataset should never be modified
	const ForestBuilderParameters& m_Parameters; // Parameters also should never be modified
	bool m_isTreeTrained;

    public:
	DecisionTreeBuilder(const ForestBuilderParameters& Parameters)
	    : m_Parameters(Parameters)
	    , m_isTreeTrained(false)
	{

	};

	bool Build(const AbstractDataSet& DataSet)
	{
	    m_Tree = std::shared_ptr<DecisionTree<T, S, R>>(new DecisionTree<T, S, R>(m_Parameters.m_MaxLevels));
	    m_DataSet = DataSet;
	    bool Success = true;
	    if(m_Parameters.m_TrainMethod == TrainMethod::DFS)
		Success = BuildTreeDepthFirst(m_DataSet, 0, 0);
	    if(m_Parameters.m_TrainMethod == TrainMethod::BFS)
		Success = BuildTreeBreadthFirst();
	    if(m_Parameters.m_TrainMethod == TrainMethod::Hybrid)
		Success = BuildTreeHybrid();

	    m_isTreeTrained = Success;
	    return m_isTreeTrained;
	};

	bool BuildTreeDepthFirst(const AbstractDataSet& PartitionedDataSet, int NodeIndex, int CurrentNodeDepth)
	{
	    // if(CurrentNodeDepth > m_Tree->GetMaxDecisionLevels())
	    // {
	    // 	m_Tree->GetNode(NodeIndex).MakeLeafNode();
	    // 	return;
	    // }

	    // // Initialize optimal values
	    // VPFloat OptGain = -std::numeric_limits<VPFloat>::infinity();
	    // T OptFeatureResponse; // This should create empty feature response
	    
	    // for(int i = 0; i < m_Parameters.m_NumCandidateThresholds; ++i)
	    // {
	    // 	T FeatureResponse(1); // This should sample from possible elements in the feature response set
	    // };	    
	};

	bool BuildTreeBreadthFirst(void)
	{
	    // TODO:
	};

	bool BuildTreeHybrid(void)
	{
	    // TODO:	    
	};

	const std::shared_ptr<DecisionTree<T, S, R>> GetTree(void) { return m_Tree; };
	const bool DoneBuild(void) { return m_isTreeTrained; };
    };
} // namespace Kaadugal

#endif // _DECISIONTREEBUILDER_HPP_
