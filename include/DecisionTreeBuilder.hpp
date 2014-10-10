#ifndef _DECISIONTREEBUILDER_HPP_
#define _DECISIONTREEBUILDER_HPP_

#include <memory>

#include "DecisionTree.hpp"
#include "Abstract/AbstractDataSet.hpp"
#include "Parameters.hpp"
#include "DataSetIndex.hpp"
#include "Randomizer.hpp"

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

	std::pair<std::shared_ptr<DataSetIndex>, std::shared_ptr<DataSetIndex>> Partition(std::shared_ptr<DataSetIndex> ParentDataSetIdx, const std::vector<VPFloat>& Responses, VPFloat Threshold)
	{
	    std::vector<int> LeftSubsetPts;
	    std::vector<int> RightSubsetPts;
	    for(int i = 0; i < ParentDataSetIdx->Size(); ++i)
	    {
		if(Responses[i] > Threshold)
		    LeftSubsetPts.push_back(ParentDataSetIdx->GetDataPointIndex(i));
		else
		    RightSubsetPts.push_back(ParentDataSetIdx->GetDataPointIndex(i));
	    }

	    return std::make_pair(std::shared_ptr<DataSetIndex>(new DataSetIndex(ParentDataSetIdx->GetDataSet(), LeftSubsetPts))
				  , std::shared_ptr<DataSetIndex>(new DataSetIndex(ParentDataSetIdx->GetDataSet(), RightSubsetPts)));
	};

	const std::vector<VPFloat> SelectThresholds(std::shared_ptr<DataSetIndex> DataSubsetIdx, const std::vector<VPFloat>& Responses)
	{
	    // Please see Efficient Implementation of Decision Forests, Shotton et al. 2013
	    // Section 21.3.3 explains how to implement this threshold selection using quantiles
	    // Also see the Sherwood Library from Microsoft Research
	    std::vector<VPFloat> Thresholds(m_Parameters.m_NumCandidateThresholds); // This is different from the Sherwood implementation. We don't use n+1
	    std::vector<int>::size_type NumThresholds;
	    std::vector<VPFloat> Quantiles(m_Parameters.m_NumCandidateThresholds + 1); // TODO: Candidate for memory saving

	    // This isn't ideal because if size of data subset is only a few above NumThresh, then Randomizer will repeat some values
	    if(DataSubsetIdx->Size() > m_Parameters.m_NumCandidateThresholds)
	    {
		// Sample m_NumCandidateThresholds+1 times (uniformly randomly) from Responses
		std::uniform_int_distribution<int> UniDist(0, int(DataSubsetIdx->Size()-1)); // Both inclusive
		for(int i = 0; i < m_Parameters.m_NumCandidateThresholds + 1; i++)
		    Quantiles[i] = Responses[UniDist(Randomizer::Get().GetRNG())];
	    }
	    else
	    {
		Quantiles.resize(Responses.size());
		std::copy(Responses.begin(), Responses.end(), Quantiles.begin());
	    }

	    // Now compute quantiles. See https://www.stat.auckland.ac.nz/~ihaka/787/lectures-quantiles-handouts.pdf
	    // if you don't know how to do this
	    std::sort(Quantiles.begin(), Quantiles.end());

	    if(Quantiles[0] == Quantiles[Quantiles.size()-1])
		return std::vector<VPFloat>(); // Looks like samples were all the same. This is bad

	    // Compute n candidate thresholds by sampling in between n+1 approximate quantiles
	    std::uniform_real_distribution<VPFloat> UniRealDist(0, 1); // [0, 1), NOTE the exclusive end
	    for(int i = 0; i < NumThresholds; ++i)
		Thresholds[i] = Quantiles[i] + VPFloat(UniRealDist(Randomizer::Get().GetRNG()) * (Quantiles[i + 1] - Quantiles[i]));

	    return Thresholds;
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
	    S ParentNodeStats(PartitionedDataSetIdx);
	    if(CurrentNodeDepth >= m_Tree->GetMaxDecisionLevels()) // Both are zero-indexed
	    {
		std::cout << "[ INFO ]: Terminating splitting at maximum tree depth." << std::endl;
	    	m_Tree->GetNode(NodeIndex).MakeLeafNode(ParentNodeStats); // Leaf node can be "endowed" with arbitrary data. TODO: Need to handle arbitrary leaf data
	    	return true;
	    }

	    // Initialize optimal values
	    VPFloat OptObjVal = 0.0;
	    T OptFeatureResponse; // This should create empty feature response
	    VPFloat OptThreshold = std::numeric_limits<VPFloat>::epsilon(); // Is this the best way?
	    std::shared_ptr<DataSetIndex> OptLeftPartitionIdx;
	    std::shared_ptr<DataSetIndex> OptRightPartitionIdx;
	    S OptLeftNodeStats;
	    S OptRightNodeStats;

	    // TODO: Candidate for parallelization
	    for(int i = 0; i < m_Parameters.m_NumCandidateFeatures; ++i)
	    {
		T FeatureResponse; // This should sample from possible elements in the feature response set
		std::vector<VPFloat> Responses;
		int DataSetSize = PartitionedDataSetIdx->Size();
		for(int k = 0; k < DataSetSize; ++k)
		    Responses.push_back(FeatureResponse.GetResponse(PartitionedDataSetIdx->GetDataPoint(k))); // TODO: Can be parallelized/made more efficient?

		const std::vector<VPFloat>& Thresholds = SelectThresholds(PartitionedDataSetIdx, Responses);
		int NumThresholds = Thresholds.size();
		
		for(int j = 0; j < NumThresholds; ++j)
		{
		    // First partition data based on current splitting candidates
		    std::pair<std::shared_ptr<DataSetIndex>, std::shared_ptr<DataSetIndex>> Subsets = Partition(PartitionedDataSetIdx, Responses, Thresholds[i]);

		    S LeftNodeStats(Subsets.first);
		    S RightNodeStats(Subsets.second);
		    
		    // Then compute some objective function value. Examples: information gain, Geni index
		    VPFloat ObjVal = GetObjectiveValue(ParentNodeStats, LeftNodeStats, RightNodeStats);

		    if(ObjVal > OptObjVal)
		    {
			OptObjVal = ObjVal;
			OptFeatureResponse = FeatureResponse;
			OptThreshold = Thresholds[i];
			OptLeftPartitionIdx = Subsets.first;
			OptRightPartitionIdx = Subsets.second;
			OptLeftNodeStats = LeftNodeStats;
			OptRightNodeStats = RightNodeStats;
		    }
		}
	    }

	    // Check for some recursion termination conditions
	    // 1. No gain
	    if(OptObjVal <= 0.0)
	    {
		// Zero gain here which is bad. 
		std::cout << "[ WARN ]: No gain for any of the splitting candidates" << std::endl;
	    	m_Tree->GetNode(NodeIndex).MakeLeafNode(ParentNodeStats); // Leaf node can be "endowed" with arbitrary data. TODO: Need to handle arbitrary leaf data
		return true;
	    }

	    // 2. TODO: Stop recursion if gain is too little

	    // Now free to make a split node
	    m_Tree->GetNode(NodeIndex).MakeSplitNode(ParentNodeStats, OptFeatureResponse, OptThreshold);
	    std::cout << "[ INFO ]: Creating split node..." << std::endl;

	    // Now recurse :)
	    // Since we store the decision tree as a full binary tree (in
	    // breadth-first order) we can easily get the left and right children indices
	    BuildTreeDepthFirst(OptLeftPartitionIdx, 2*NodeIndex+1, CurrentNodeDepth+1);
	    BuildTreeDepthFirst(OptRightPartitionIdx, 2*NodeIndex+2, CurrentNodeDepth+1);
	    	    
	    return true;
	};

	VPFloat GetObjectiveValue(S& ParentStats, S& LeftStats, S& RightStats)
	{
	    // Statistics are already aggregated
	    if(!ParentStats.isAggregated() || !LeftStats.isAggregated() || !RightStats.isAggregated())
		return 0.0;

	    // NOTE: We are using information gain as the objective function
	    // Change this and use a template/abstract class, if needed
	    // See any of the Shotton et al. papers for this definition
	    VPFloat InformationGain = ParentStats.GetEntropy()
		- ( VPFloat(LeftStats.GetNumDataPoints())  * LeftStats.GetEntropy()
		    + VPFloat(RightStats.GetNumDataPoints()) * RightStats.GetEntropy() ) / VPFloat(ParentStats.GetNumDataPoints());

	    return InformationGain;
	};

	bool BuildTreeBreadthFirst(void)
	{
	    // TODO:
	    std::cout << "[ INFO ]: BuildTreeBreadthFirst() is not yet implemented." << std::endl;
	};

	bool BuildTreeHybrid(void)
	{
	    // TODO:
	    std::cout << "[ INFO ]: BuildTreeHybrid() is not yet implemented." << std::endl;
	};

	const std::shared_ptr<DecisionTree<T, S, R>> GetTree(void) { return m_Tree; };
	const bool DoneBuild(void) { return m_isTreeTrained; };
    };
} // namespace Kaadugal

#endif // _DECISIONTREEBUILDER_HPP_
