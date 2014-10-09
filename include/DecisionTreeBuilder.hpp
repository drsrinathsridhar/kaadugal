#ifndef _DECISIONTREEBUILDER_HPP_
#define _DECISIONTREEBUILDER_HPP_

#include <memory>

#include "DecisionTree.hpp"
#include "Abstract/AbstractDataSet.hpp"
#include "Parameters.hpp"
#include "DataSetIndex.hpp"

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
	std::shared_ptr<DataSetIndex> m_PartitionedDataSetIdx;
	const ForestBuilderParameters& m_Parameters; // Parameters also should never be modified
	bool m_isTreeTrained;

    public:
	DecisionTreeBuilder(const ForestBuilderParameters& Parameters)
	    : m_Parameters(Parameters)
	    , m_isTreeTrained(false)
	{

	};

	bool Build(std::shared_ptr<DataSetIndex> PartitionedDataSetIdx)
	{
	    m_Tree = std::shared_ptr<DecisionTree<T, S, R>>(new DecisionTree<T, S, R>(m_Parameters.m_MaxLevels));
	    m_PartitionedDataSetIdx = PartitionedDataSetIdx;
	    bool Success = true;
	    if(m_Parameters.m_TrainMethod == TrainMethod::DFS)
		Success = BuildTreeDepthFirst(m_PartitionedDataSetIdx, 0, 0);
	    if(m_Parameters.m_TrainMethod == TrainMethod::BFS)
		Success = BuildTreeBreadthFirst();
	    if(m_Parameters.m_TrainMethod == TrainMethod::Hybrid)
		Success = BuildTreeHybrid();

	    m_isTreeTrained = Success;
	    return m_isTreeTrained;
	};

	bool BuildTreeDepthFirst(std::shared_ptr<DataSetIndex> PartitionedDataSetIdx, int NodeIndex, int CurrentNodeDepth)
	{
	    if(CurrentNodeDepth > m_Tree->GetMaxDecisionLevels())
	    {
	    	// m_Tree->GetNode(NodeIndex).MakeLeafNode(); // Leaf node can be "endowed" with arbitrary data
	    	return true;
	    }

	    // Initialize optimal values
	    VPFloat OptGain = 0.0;
	    T OptFeatureResponse; // This should create empty feature response
	    VPFloat OptThreshold = std::numeric_limits<VPFloat>::epsilon(); // Is this the best way?
	    std::shared_ptr<DataSetIndex> OptLeftPartition;
	    std::shared_ptr<DataSetIndex> OptRightPartition;

	    int NumThresholds = 100; // TODO: Need an intelligent way of finding this
	    std::vector<VPFloat> Thresholds;
  	    
	    for(int i = 0; i < m_Parameters.m_NumCandidateThresholds; ++i)
	    {
		T FeatureResponse; // This should sample from possible elements in the feature response set
		
		for(int j = 0; j < NumThresholds; ++j)
		{
		    // First partition data based on current splitting candidates
		    // TODO:
		    std::pair<std::shared_ptr<DataSetIndex>, std::shared_ptr<DataSetIndex>> Subsets;
		    // std::pair<AbstractDataSet, AbstractDataSet> Subsets = PartitionedDataSet.Partition(FeatureReponse, Thresholds[i]);
		    
		    // Then compute information gain
		    // TODO:
		    VPFloat Gain;
		    // VPFloat Gain = GetInfoGain(PartitionedDataSet, Subsets.first, Subsets.second); // Can use other criteria like Geni index if needed

		    if(Gain > OptGain)
		    {
			OptGain = Gain;
			OptFeatureResponse = FeatureResponse;
			OptThreshold = Thresholds[i];
			OptLeftPartition = Subsets.first;
			OptRightPartition = Subsets.second;
		    }
		}
	    }

	    // Check for some recursion termination conditions
	    // 1. No gain
	    if(OptGain <= 0.0)
	    {
		// TODO: Build leaf node
		// Zero gain here which is bad. 
		std::cout << "[ WARN ]: No gain for any of the splitting candidates";
		return true;
	    }

	    // 2. TODO: Stop recursion if gain is too little

	    // Now free to make a split node
	    // TODO: Make a split node from splitting candidates, data and statistics

	    // Now recurse ;)
	    // Since we store the decision tree as a full binary tree (in
	    // breadth-first order) we can easily get the left and right children indices
	    BuildTreeDepthFirst(OptLeftPartition, 2*NodeIndex+1, CurrentNodeDepth+1);
	    BuildTreeDepthFirst(OptRightPartition, 2*NodeIndex+2, CurrentNodeDepth+1);
	    	    
	    return true;
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
