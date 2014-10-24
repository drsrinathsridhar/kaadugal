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
    template<class T, class S, class R = AbstractLeafData>
    class DecisionTreeBuilder
    {
    private:
	std::shared_ptr<DecisionTree<T, S, R>> m_Tree;
	// std::shared_ptr<DataSetIndex> m_PartitionedDataSetIdx;
	const ForestBuilderParameters& m_Parameters; // Parameters also should never be modified
	bool m_isTreeTrained;

	// Members for bread-first building
	std::vector<int> m_FrontierIdx; // Is not strictly the frontier but a subset with all non-built nodes
	std::vector<int> m_DataDeepestNodeIndex; // Stores the node index of the (currently) lowest node that a data point reaches. Same size as the number of data points

	// When training tree separately, we need a pointer to the data
	std::shared_ptr<AbstractDataSet> m_DataSet;

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
		if(Responses[i] > Threshold) // Please use same logic when testing the tree
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
		Thresholds.resize(Responses.size()-1); // One less than quantiles
		std::copy(Responses.begin(), Responses.end(), Quantiles.begin());
	    }

	    // Now compute quantiles. See https://www.stat.auckland.ac.nz/~ihaka/787/lectures-quantiles-handouts.pdf
	    // if you don't know how to do this
	    std::sort(Quantiles.begin(), Quantiles.end());

	    if(Quantiles[0] == Quantiles[Quantiles.size()-1])
		return std::vector<VPFloat>(); // Looks like samples were all the same. This is bad

	    // Compute n candidate thresholds by sampling in between n+1 approximate quantiles
	    std::uniform_real_distribution<VPFloat> UniRealDist(0, 1); // [0, 1), NOTE the exclusive end
	    int NumThresholds = Thresholds.size();
	    for(int i = 0; i < NumThresholds; ++i)
		Thresholds[i] = Quantiles[i] + VPFloat(UniRealDist(Randomizer::Get().GetRNG()) * (Quantiles[i + 1] - Quantiles[i]));

	    // std::cout << "Before return:\n";
	    // for(int j = 0; j < Thresholds.size(); ++j)
	    // 	std::cout << "Thresh: " << Thresholds[j] << std::endl;

	    return Thresholds;
	};

	bool Build(std::shared_ptr<DataSetIndex> PartitionedDataSetIdx)
	{
	    m_Tree = std::shared_ptr<DecisionTree<T, S, R>>(new DecisionTree<T, S, R>(m_Parameters.m_MaxLevels));
	    bool Success = true;
	    if(m_Parameters.m_TrainMethod == TrainMethod::DFS)
	    	Success = BuildTreeDepthFirst(PartitionedDataSetIdx, 0, 0);
	    if(m_Parameters.m_TrainMethod == TrainMethod::BFS)
	    	Success = BuildTreeBreadthFirst(PartitionedDataSetIdx);
	    if(m_Parameters.m_TrainMethod == TrainMethod::Hybrid)
	    	Success = BuildTreeHybrid();

	    m_isTreeTrained = Success;
	    return m_isTreeTrained;
	};

	// Overloaded version if we would like to build only one tree at a time
	bool Build(std::shared_ptr<AbstractDataSet> DataSet)
	{
	    m_DataSet = DataSet;
	    int SetSize = m_DataSet->Size();
	    if(SetSize <= 1)
	    {
		std::cout << "[ WARN ]: The number of training samples (" << SetSize << ") is too low. Cannot train this tree." << std::endl;
		return false;
	    }

	    // Randomize the data
	    // Create an indices set with all indices
	    std::vector<int> Indices;
	    for(int i = 0; i < SetSize; ++i)
		Indices.push_back(i);
	    std::shuffle(Indices.begin(), Indices.end(), Randomizer::Get().GetRNG());
	    
	    // Contains index to all points in the data set BUT they are randomized
	    m_isTreeTrained = Build(std::shared_ptr<DataSetIndex>(new DataSetIndex(m_DataSet, Indices)));
	    return m_isTreeTrained;
	};

	bool BuildTreeDepthFirst(std::shared_ptr<DataSetIndex> PartitionedDataSetIdx, int NodeIndex, int CurrentNodeDepth)
	{
	    std::cout << "[ INFO ]: At depth: " << CurrentNodeDepth << std::endl;
	    S ParentNodeStats(PartitionedDataSetIdx);
	    // std::cout << ParentNodeStats.GetNumDataPoints() << std::endl;
	    // std::cout << ParentNodeStats.GetProbability(0) << std::endl;
	    // std::cout << ParentNodeStats.FindWinnerLabelIndex() << std::endl;
	    // return true;

	    if(CurrentNodeDepth >= m_Tree->GetMaxDecisionLevels()) // Both are zero-indexed
	    {
		std::cout << "[ INFO ]: Terminating splitting at maximum tree depth." << std::endl;
	    	m_Tree->GetNode(NodeIndex).MakeLeafNode(ParentNodeStats); // Leaf node can be "endowed" with arbitrary data. TODO: Need to handle arbitrary leaf data
	    	return true;
	    }

	    // Initialize optimal values
	    VPFloat OptObjVal = 0.0;
	    T OptFeatureResponse; // This creates an empty feature response with random response
	    VPFloat OptThreshold = 0.0;
	    std::shared_ptr<DataSetIndex> OptLeftPartitionIdx;
	    std::shared_ptr<DataSetIndex> OptRightPartitionIdx;
	    S OptLeftNodeStats;
	    S OptRightNodeStats;

	    // TODO: Candidate for parallelization
	    for(int i = 0; i < m_Parameters.m_NumCandidateFeatures; ++i)
	    {
		T FeatureResponse; // This creates an empty feature response with random response
		std::vector<VPFloat> Responses;
		int DataSetSize = PartitionedDataSetIdx->Size();
		for(int k = 0; k < DataSetSize; ++k)
		    Responses.push_back(FeatureResponse.GetResponse(PartitionedDataSetIdx->GetDataPoint(k))); // TODO: Can be parallelized/made more efficient?

		const std::vector<VPFloat>& Thresholds = SelectThresholds(PartitionedDataSetIdx, Responses);
		int NumThresholds = Thresholds.size();
		// for(int j = 0; j < NumThresholds; ++j)
		//     std::cout << Thresholds[j] << "\t";
		// std::cout << std::endl;
		
		for(int j = 0; j < NumThresholds; ++j)
		{
		    // First partition data based on current splitting candidates
		    std::pair<std::shared_ptr<DataSetIndex>, std::shared_ptr<DataSetIndex>> Subsets = Partition(PartitionedDataSetIdx, Responses, Thresholds[j]);
		    // if(Subsets.first->Size() == 0 || Subsets.second->Size() == 0)
		    // {
		    // 	std::cout << "Parent size: " << PartitionedDataSetIdx->Size() << std::endl;
		    // 	std::cout << "Left size: " << Subsets.first->Size() << std::endl;
		    // 	std::cout << "Right size: " << Subsets.second->Size() << std::endl;
		    // 	std::cout << "Problem: " << j << ", " << i << "\n";
		    // 	return false;
		    // }

		    S LeftNodeStats(Subsets.first);
		    S RightNodeStats(Subsets.second);
		    
		    // Then compute some objective function value. Examples: information gain, Geni index
		    VPFloat ObjVal = GetObjectiveValue(ParentNodeStats, LeftNodeStats, RightNodeStats);

		    if(ObjVal >= OptObjVal)
		    {
			OptObjVal = ObjVal;
			OptFeatureResponse = FeatureResponse;
			OptThreshold = Thresholds[j];
			OptLeftPartitionIdx = Subsets.first;
			OptRightPartitionIdx = Subsets.second;
			OptLeftNodeStats = LeftNodeStats; // TODO: Overload = operator
			OptRightNodeStats = RightNodeStats;
		    }
		}
	    }

	    // std::cout << "\n--------------------------------\n" << "Depth Level: " << CurrentNodeDepth << "\n--------------------------------\n";
	    // {
	    // 	std::cout << "Parent size: " << PartitionedDataSetIdx->Size() << std::endl;
	    // 	std::cout << "Left size: " << OptLeftPartitionIdx->Size() << std::endl;
	    // 	std::cout << "Right size: " << OptRightPartitionIdx->Size() << std::endl;
	    // 	std::cout << "L0: " << OptLeftNodeStats.GetProbability(0) << std::endl;
	    // 	std::cout << "L1: " << OptLeftNodeStats.GetProbability(1) << std::endl;
	    // 	std::cout << "L2: " << OptLeftNodeStats.GetProbability(2) << std::endl;
	    // 	std::cout << "L3: " << OptLeftNodeStats.GetProbability(3) << std::endl;

	    // 	std::cout << std::endl;
		
	    // 	std::cout << "R0: " << OptRightNodeStats.GetProbability(0) << std::endl;
	    // 	std::cout << "R1: " << OptRightNodeStats.GetProbability(1) << std::endl;
	    // 	std::cout << "R2: " << OptRightNodeStats.GetProbability(2) << std::endl;
	    // 	std::cout << "R3: " << OptRightNodeStats.GetProbability(3) << std::endl;

	    // 	std::cout << "\nOptGain: " << OptObjVal << std::endl;
	    // 	std::cout << "OptFeatResponse: " << OptFeatureResponse.GetSelectedFeature() << std::endl;
	    // 	std::cout << "OptThreshold: " << OptThreshold << std::endl;
	    // }

	    // Check for some recursion termination conditions
	    // No gain or very small gain
	    if(OptObjVal == 0.0 || OptObjVal < m_Parameters.m_MinGain)
	    {
		std::cout << "[ INFO ]: No gain or very small gain (" << OptObjVal << ") for all splitting candidates. Making leaf node..." << std::endl;
	    	m_Tree->GetNode(NodeIndex).MakeLeafNode(ParentNodeStats); // Leaf node can be "endowed" with arbitrary data. TODO: Need to handle arbitrary leaf data
		return true;
	    }

	    // Now free to make a split node
	    m_Tree->GetNode(NodeIndex).MakeSplitNode(ParentNodeStats, OptFeatureResponse, OptThreshold);
	    std::cout << "[ INFO ]: Creating split node..." << std::endl;

	    // Now recurse :)
	    // Since we store the decision tree as a full binary tree (in
	    // breadth-first order) we can easily get the left and right children indices
	    if(OptLeftPartitionIdx->Size() > 0)
		BuildTreeDepthFirst(OptLeftPartitionIdx, 2*NodeIndex+1, CurrentNodeDepth+1);
	    else
		std::cout << "[ WARN ]: No data so not recursing." << std::endl;

	    if(OptRightPartitionIdx->Size() > 0)
		BuildTreeDepthFirst(OptRightPartitionIdx, 2*NodeIndex+2, CurrentNodeDepth+1);
	    else
		std::cout << "[ WARN ]: No data so not recursing." << std::endl;
	    	    
	    return true;
	};

	VPFloat GetObjectiveValue(S& ParentStats, S& LeftStats, S& RightStats)
	{
	    // Statistics are already aggregated
	    // TODO: What if they are not aggregated?

	    if(ParentStats.GetNumDataPoints() <= 0)
		return 0.0;

	    // NOTE: We are using information gain as the objective function
	    // Change this and use a template/abstract class, if needed
	    // See any of the Shotton et al. papers for this definition
	    VPFloat InformationGain = ParentStats.GetEntropy()
		- ( VPFloat(LeftStats.GetNumDataPoints())  * LeftStats.GetEntropy()
		    + VPFloat(RightStats.GetNumDataPoints()) * RightStats.GetEntropy() ) / VPFloat(ParentStats.GetNumDataPoints());

	    // std::cout << "InfoGain: " << InformationGain << std::endl;

	    return InformationGain;
	};

	bool BuildTreeBreadthFirst(std::shared_ptr<DataSetIndex> DataSetIdx)
	{
	    // All the incoming data reaches the root for sure
	    m_DataDeepestNodeIndex.resize(DataSetIdx->Size(), 0); // Later this is updated inside BuildTreeFrontier()
	    UpdateFrontierIdx(); // Update before starting. Later this is called inside BuildTreeFrontier()

	    for(int i = 0; i < m_Tree->GetMaxDecisionLevels(); ++i)
		BuildTreeFrontier(DataSetIdx);
	    
	    return true;
	};

	void BuildTreeFrontier(std::shared_ptr<DataSetIndex> DataSetIdx)
	{
	    // TODO: Convert int to vector::type to avoid int overflow problems
	    // Using large integers
	    int64_t NumCandidateThresholds = m_Parameters.m_NumCandidateThresholds>DataSetIdx->Size()?m_Parameters.m_NumCandidateThresholds:DataSetIdx->Size();
	    int64_t NumSplitCandidates = m_Parameters.m_NumCandidateFeatures * NumCandidateThresholds;
	    int64_t NumFrontierNodes = m_FrontierIdx.size();
	    std::vector<S> AllStatistics(NumSplitCandidates*NumFrontierNodes);
	    // NOTE: This is different from Criminisi et al. We create a 3D matrix here.
	    // Also we use a single vector to denote the 3D matrix so that they are all contiguous in memory (heap as all vectors are)
	    // Dimensions are Rows (i, FeatureResponses) x Cols (j, Thresholds) x Depth (k, Frontier nodes)
	    std::vector<T> FeatureResponses(NumSplitCandidates*NumFrontierNodes);

	    // Doing the two-pass strategy described in Efficient Implementation of Decision Forests by Criminisi et al.
	    // The first pass gives us proper candidate thresholds which are used in the second pass
	    int DataSetSize = DataSetIdx->Size();
	    for(int DatItr = 0; DatItr < DataSetSize; ++DatItr)
	    {
		// // First find which node this data point reaches
		// int k = m_DataDeepestNodeIndex[DatItr];
		// if(std::find(m_FrontierIdx.begin(), m_FrontierIdx.end(), k) == m_FrontierIdx.end())
		//     continue; // Since the above k might not be in the "frontier", continue to the next data point

		// for(int FeatCtr = 0; FeatCtr < m_Parameters.m_NumCandidateFeatures; ++FeatCtr)
		// {
		//     T FeatureResponse; // This creates an empty feature response with random response
		//     std::vector<VPFloat> Responses;
		//     int DataSetSize = PartitionedDataSetIdx->Size();
		//     for(int k = 0; k < DataSetSize; ++k)
		// 	Responses.push_back(FeatureResponse.GetResponse(PartitionedDataSetIdx->GetDataPoint(k))); // TODO: Can be parallelized/made more efficient?

		//     const std::vector<VPFloat>& Thresholds = SelectThresholds(PartitionedDataSetIdx, Responses);
		//     int NumThresholds = Thresholds.size();
		// }
	    }
	};

	void UpdateFrontierIdx(void)
	{
	    if(m_FrontierIdx.size() == 0) // First iteration, the frontier has nothing so we add the root to the frontier
	    {
		m_FrontierIdx.push_back(0);
		return;
	    }

	    std::vector<int> LocalFrontIdx = m_FrontierIdx;
	    m_FrontierIdx.clear();
	    for(auto itr = LocalFrontIdx.begin(); itr != LocalFrontIdx.end(); ++itr)
	    {
		int NodeIndex = (*itr);
		if(m_Tree->GetNode(NodeIndex).GetType() == Kaadugal::NodeType::Invalid)
		{
		    std::cout << "[ WARN ]: Something is not right. Frontier has an invalid node. Frontier not changed." << std::endl;
		    m_FrontierIdx = LocalFrontIdx;
		    return;
		}
		
		if(m_Tree->GetNode(NodeIndex).GetType() == Kaadugal::NodeType::LeafNode)
		{
		    // Do nothing, since this is a frontier node that would have been built already, we don't need this in the frontier
		}

		if(m_Tree->GetNode(NodeIndex).GetType() == Kaadugal::NodeType::SplitNode)
		{
		    // Add it's two children to the frontier
		    m_FrontierIdx.push_back(2*NodeIndex + 1); // Left child
		    m_FrontierIdx.push_back(2*NodeIndex + 2); // Right child
		}
	    }
	};

	bool BuildTreeHybrid(void)
	{
	    // TODO:
	    std::cout << "[ INFO ]: BuildTreeHybrid() is not yet implemented." << std::endl;
	    return false;
	};

	std::shared_ptr<DecisionTree<T, S, R>> GetTree(void) { return m_Tree; };
	bool DoneBuild(void) { return m_isTreeTrained; };
    };
} // namespace Kaadugal

#endif // _DECISIONTREEBUILDER_HPP_
