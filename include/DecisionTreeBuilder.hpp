#ifndef _DECISIONTREEBUILDER_HPP_
#define _DECISIONTREEBUILDER_HPP_

#include <memory>
#include <map>
#include <omp.h>
#include <chrono> // This does not work with MSVC 2010 or GCC old versions

#include "DecisionTree.hpp"
#include "Abstract/AbstractDataSet.hpp"
#include "Parameters.hpp"
#include "DataSetIndex.hpp"
#include "Randomizer.hpp"
#include "Utilities.hpp"

// TODO: Avoid using push_back()?
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
		std::vector<int> m_FrontierNodes; // Is not strictly the frontier but a subset with all non-built nodes
		std::vector<int> m_DataDeepestNodeIndex; // Stores the node index of the (currently) lowest node that a data point reaches. Same size as the number of data points

		// When training tree separately, we need a pointer to the data
		std::shared_ptr<AbstractDataSet> m_DataSet;

		// Useful for printing the depth that the tree has reached
		int m_ReachedMaxDepth;
		uint64_t m_TimeStartedBuild;
		uint64_t m_TimeFinishedBuild;
		std::vector<VPFloat> m_TreeLevelTimes; // Store time for training each tree level

		// Structure needed for OpenMP accumulator variable
		struct OptParamsStruct
		{
		public:
			OptParamsStruct(void)
			{
				s_isValid = false;
			};

			OptParamsStruct(const VPFloat& Thresh, const T& FeatureResponse, const bool isValid)
				: s_Threshold(Thresh)
				, s_FeatureResponse(FeatureResponse)
				, s_isValid(isValid)
			{

			};
			VPFloat s_Threshold;
			T s_FeatureResponse;
			bool s_isValid;
		};

	public:
		int m_NumLeafNodes;
		int m_NumSplitNodes;

		DecisionTreeBuilder(const ForestBuilderParameters& Parameters)
			: m_Parameters(Parameters)
			, m_isTreeTrained(false)
			, m_ReachedMaxDepth(0)
			, m_NumLeafNodes(0)
			, m_NumSplitNodes(0)
		{
			m_TreeLevelTimes.resize(m_Parameters.m_MaxLevels + 1, 0.0); // Zero indexed. 0.0 time means that level was never reached
		};

		bool Build(std::shared_ptr<DataSetIndex> PartitionedDataSetIdx)
		{
			m_Tree = std::shared_ptr<DecisionTree<T, S, R>>(new DecisionTree<T, S, R>(m_Parameters.m_MaxLevels));
			bool Success = true;
			m_TimeStartedBuild = GetCurrentEpochTime();
			if (m_Parameters.m_TrainMethod == TrainMethod::DFS)
			{
				std::cout << "[ INFO ]: At depth: " << std::flush;
				Success = BuildTreeDepthFirst(PartitionedDataSetIdx, 0, 0);
			}
			if (m_Parameters.m_TrainMethod == TrainMethod::BFS)
				Success = BuildTreeBreadthFirst(PartitionedDataSetIdx);
			if (m_Parameters.m_TrainMethod == TrainMethod::Hybrid)
				Success = BuildTreeHybrid();

			m_TimeFinishedBuild = GetCurrentEpochTime();
			std::cout << ": Finished in " << (m_TimeFinishedBuild - m_TimeStartedBuild) * 1e-6 << " s." << std::endl;
			std::cout << "[ INFO ]: Times at each level (seconds) " << std::endl;

			VPFloat TotalTime = 0.0;
			for (unsigned int i = 0; i < m_TreeLevelTimes.size(); ++i)
			{
				VPFloat LevelTime = VPFloat(m_TreeLevelTimes[i]) * 1e-6;
				TotalTime += LevelTime;
				std::cout << LevelTime << " ";
			}
			std::cout << " (Total = " << TotalTime << " s)" << std::endl;

			m_isTreeTrained = Success;
			return m_isTreeTrained;
		};

		// Overloaded version if we would like to build only one tree at a time
		bool Build(std::shared_ptr<AbstractDataSet> DataSet)
		{
			m_DataSet = DataSet;
			int SetSize = m_DataSet->Size();
			if (SetSize <= 1)
			{
				std::cout << "[ WARN ]: The number of training samples (" << SetSize << ") is too low. Cannot train this tree." << std::endl;
				return false;
			}

			// Randomize the data
			// Create an indices set with all indices
			std::vector<int> Indices;
			for (int i = 0; i < SetSize; ++i)
				Indices.push_back(i);
			std::shuffle(Indices.begin(), Indices.end(), Randomizer::Get().GetRNG());

			// Contains index to all points in the data set BUT they are randomized
			m_isTreeTrained = Build(std::shared_ptr<DataSetIndex>(new DataSetIndex(m_DataSet, Indices)));
			return m_isTreeTrained;
		};

		bool BuildTreeDepthFirst(std::shared_ptr<DataSetIndex> PartitionedDataSetIdx, int NodeIndex, int CurrentNodeDepth)
		{
			if (m_ReachedMaxDepth < CurrentNodeDepth)
			{
				m_ReachedMaxDepth = CurrentNodeDepth;
				std::cout << CurrentNodeDepth << " " << std::flush;
			}

			// Start time for node time computation
			uint64_t NodeStartTime = GetCurrentEpochTime();

			S ParentNodeStats(PartitionedDataSetIdx);
			int DataSetSize = PartitionedDataSetIdx->Size();
			// std::cout << ParentNodeStats.GetNumDataPoints() << std::endl;
			// std::cout << ParentNodeStats.GetProbability(0) << std::endl;

			// Check if incoming data is fewer than 3 data points. If so then just create a leaf node
			// It's 3 (instead of 2) because otherwise the SelectThresholds() could return 1 which is problematic
			if (DataSetSize < std::max(3, m_Parameters.m_MinDataSetSize))
			{
				//std::cout << "[ INFO ]: Fewer than 2 data points in reached this node. Making leaf node..." << std::endl;
				m_Tree->GetNode(NodeIndex).MakeLeafNode(ParentNodeStats); // Leaf node can be "endowed" with arbitrary data. TODO: Need to handle arbitrary leaf data
				PartitionedDataSetIdx->GetDataSet()->Special(NodeIndex, PartitionedDataSetIdx->GetIndex());
				uint64_t NodeEndTime = GetCurrentEpochTime();
				m_TreeLevelTimes[CurrentNodeDepth] += NodeEndTime - NodeStartTime;
				m_NumLeafNodes++;

				return true;
			}

			if (CurrentNodeDepth >= m_Tree->GetMaxDecisionLevels()) // Both are zero-indexed
			{
				//std::cout << "[ INFO ]: Terminating splitting at maximum tree depth." << std::endl;
				m_Tree->GetNode(NodeIndex).MakeLeafNode(ParentNodeStats); // Leaf node can be "endowed" with arbitrary data. TODO: Need to handle arbitrary leaf data
				PartitionedDataSetIdx->GetDataSet()->Special(NodeIndex, PartitionedDataSetIdx->GetIndex());
				uint64_t NodeEndTime = GetCurrentEpochTime();
				m_TreeLevelTimes[CurrentNodeDepth] += NodeEndTime - NodeStartTime;
				m_NumLeafNodes++;

				return true;
			}

			// Initialize optimal values
			VPFloat OptObjVal = -1.0; // Negative values are not possible since this is an energy
			T OptFeatureResponse; // This creates an empty feature response with random response
			VPFloat OptThreshold = 0.0;
			std::shared_ptr<DataSetIndex> OptLeftPartitionIdx;
			std::shared_ptr<DataSetIndex> OptRightPartitionIdx;
			S OptLeftNodeStats;
			S OptRightNodeStats;

			std::vector<VPFloat> ObjValAccum(m_Parameters.m_NumCandidateFeatures, -1.0);
			std::vector<OptParamsStruct> OptParamsStructAccum(m_Parameters.m_NumCandidateFeatures);
			omp_set_dynamic(0); // Explicitly disable dynamic teams
			omp_set_num_threads(std::min(m_Parameters.m_NumThreads, omp_get_max_threads()));
#pragma omp parallel for
			for (int i = 0; i < m_Parameters.m_NumCandidateFeatures; ++i)
			{
				T FeatureResponse; // This creates an empty feature response with random response
				std::vector<VPFloat> Responses;
				Responses.resize(DataSetSize);
				for (int k = 0; k < DataSetSize; ++k)
					Responses[k] = FeatureResponse.GetResponse(PartitionedDataSetIdx->GetDataPoint(k)); // TODO: Can be parallelized/made more efficient?

				const std::vector<VPFloat>& Thresholds = SelectThresholds(Responses, PartitionedDataSetIdx->Size());
				int NumThresholds = Thresholds.size();

				VPFloat LocObjVal = -1.0;
				OptParamsStruct LocObjValStruct;
				for (int j = 0; j < NumThresholds; ++j)
				{
					// First partition data based on current splitting candidates
					std::pair<std::shared_ptr<DataSetIndex>, std::shared_ptr<DataSetIndex>> Subsets = Partition(PartitionedDataSetIdx, Responses, Thresholds[j]);

					S LeftNodeStats(Subsets.first);
					S RightNodeStats(Subsets.second);

					// Then compute some objective function value. Examples: information gain, Geni index
					VPFloat ObjVal = GetObjectiveValue(ParentNodeStats, LeftNodeStats, RightNodeStats);

					if (ObjVal > LocObjVal)
					{
						LocObjVal = ObjVal;
						LocObjValStruct = OptParamsStruct(Thresholds[j], FeatureResponse, true);
					}
				}

				ObjValAccum[i] = LocObjVal;
				OptParamsStructAccum[i] = LocObjValStruct;
			}

			int AccumSize = ObjValAccum.size();
			for (int ii = 0; ii < AccumSize; ++ii)
			{
				if (OptParamsStructAccum[ii].s_isValid == false)
					continue;

				if (ObjValAccum[ii] > OptObjVal)
				{
					OptObjVal = ObjValAccum[ii];
					OptFeatureResponse = OptParamsStructAccum[ii].s_FeatureResponse;
					OptThreshold = OptParamsStructAccum[ii].s_Threshold;
				}
			}

			if (OptObjVal < 0.0)
			{
				//std::cout << "RUNTIME ERROR in BuildTreeDepthFirst()" << std::endl; // For windows
				throw std::runtime_error("Optimum objective value is negative. Cannot proceed.");
			}

			std::vector<VPFloat> DataResponses(DataSetSize);
#pragma omp parallel for
			for (int k = 0; k < DataSetSize; ++k)
				DataResponses[k] = OptFeatureResponse.GetResponse(PartitionedDataSetIdx->GetDataPoint(k));

			std::pair<std::shared_ptr<DataSetIndex>, std::shared_ptr<DataSetIndex>> Subsets = Partition(PartitionedDataSetIdx, DataResponses, OptThreshold);
			S LeftNodeStats(Subsets.first);
			S RightNodeStats(Subsets.second);

			OptLeftPartitionIdx = Subsets.first;
			OptRightPartitionIdx = Subsets.second;
			OptLeftNodeStats = LeftNodeStats;
			OptRightNodeStats = RightNodeStats;

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

			// Check for recursion termination conditions
			// No gain or very small gain
			if (OptObjVal == 0.0 || OptObjVal < m_Parameters.m_MinGain)
			{
			    //std::cout << "[ INFO ]: No gain or very small gain (" << OptObjVal << ") for all splitting candidates. Making leaf node..." << std::endl;
				m_Tree->GetNode(NodeIndex).MakeLeafNode(ParentNodeStats); // Leaf node can be "endowed" with arbitrary data. TODO: Need to handle arbitrary leaf data
				PartitionedDataSetIdx->GetDataSet()->Special(NodeIndex, PartitionedDataSetIdx->GetIndex());
				uint64_t NodeEndTime = GetCurrentEpochTime();
				m_TreeLevelTimes[CurrentNodeDepth] += NodeEndTime - NodeStartTime;
				m_NumLeafNodes++;

				return true;
			}

			// Now free to make a split node
			m_Tree->GetNode(NodeIndex).MakeSplitNode(ParentNodeStats, OptFeatureResponse, OptThreshold);
			m_NumSplitNodes++;
			// std::cout << "[ INFO ]: Creating split node..." << std::endl;

			// Now recurse :)
			// Since we store the decision tree as a full binary tree (in
			// breadth-first order) we can easily get the left and right children indices
			bool Success = true;
			uint64_t NodeEndTime = GetCurrentEpochTime();
			m_TreeLevelTimes[CurrentNodeDepth] += NodeEndTime - NodeStartTime;

			Success &= BuildTreeDepthFirst(OptLeftPartitionIdx, 2 * NodeIndex + 1, CurrentNodeDepth + 1);
			Success &= BuildTreeDepthFirst(OptRightPartitionIdx, 2 * NodeIndex + 2, CurrentNodeDepth + 1);

			return Success;
		};

		bool BuildTreeBreadthFirst(std::shared_ptr<DataSetIndex> DataSetIdx)
		{
			// All the incoming data reaches the root for sure
			m_DataDeepestNodeIndex.resize(DataSetIdx->Size(), 0); // Later this is updated inside BuildTreeFrontier()
			UpdateFrontierNodes(); // Update before starting. Later this is called inside BuildTreeFrontier()

			std::cout << "[ INFO ]: At depth: " << std::flush;
			for (int i = 0; i < m_Tree->GetMaxDecisionLevels(); ++i)
			{
				uint64_t NodeStartTime = GetCurrentEpochTime();
				std::cout << i << " " << std::flush;
				BuildTreeFrontier(DataSetIdx);
				UpdateFrontierNodes();
				uint64_t NodeEndTime = GetCurrentEpochTime();
				m_TreeLevelTimes[i] += NodeEndTime - NodeStartTime;
			}

			return true;
		};

		void BuildTreeFrontier(std::shared_ptr<DataSetIndex> DataSetIdx)
		{
			int64_t DataSetSize = DataSetIdx->Size();

			// Check if incoming data is fewer than 3 data points at the root. If so then just create a leaf node
			// It's 3 (instead of 2) because otherwise the SelectThresholds() could return 1 which is problematic
			if (DataSetSize < std::max(3, m_Parameters.m_MinDataSetSize))
			{
				m_Tree->GetNode(0).MakeLeafNode(S(DataSetIdx)); // Leaf node can be "endowed" with arbitrary data. TODO: Need to handle arbitrary leaf data
				return;
			}

			// Using large integers - uint64_t
			int64_t NumCandidateThresholds = DataSetSize>m_Parameters.m_NumCandidateThresholds ? m_Parameters.m_NumCandidateThresholds : (DataSetSize - 1);
			int64_t NumSplitCandidates = m_Parameters.m_NumCandidateFeatures * NumCandidateThresholds;
			int64_t NumFrontierNodes = m_FrontierNodes.size();
			// NOTE: This is different from Criminisi et al. We create a 3D matrix here.
			// TODO: Most likely run out of memory if this is not done in smaller batches. Frontier must be small
			// Also we use a single vector to denote the 3D matrix so that they are all contiguous in memory (heap as all vectors are)
			// Dimensions are Rows (i, FeatureResponses) x Cols (j, Thresholds) x Depth (k, Frontier nodes)
			std::vector<S> AllParentNodeStatistics(NumFrontierNodes);
			std::vector<S> AllLeftNodeStatistics(NumSplitCandidates*NumFrontierNodes);
			std::vector<S> AllRightNodeStatistics(NumSplitCandidates*NumFrontierNodes);
			std::vector<T> AllFeatureResponses(m_Parameters.m_NumCandidateFeatures*NumFrontierNodes); // Rows are features, 3 dimension is frontier
			// TODO: Change to shared_ptr?
			std::vector<VPFloat> AllThresholds(NumSplitCandidates*NumFrontierNodes); // Rows are features, cols are thresholds, 3 dimension is frontier. It is stored column-first, row-next and 3D last
			std::vector<std::vector<VPFloat>> AllResponses(m_Parameters.m_NumCandidateFeatures*NumFrontierNodes); // Rows are features, 3 dimension is frontier
			std::vector<int> FrontierDataSize(NumFrontierNodes, 0);

			for (int kk = 0; kk < DataSetSize; ++kk)
			{
				int ReachedFrontier = m_Tree->TraverseToFrontier(DataSetIdx->GetDataPoint(kk)); // Find which node this data point reaches
				if (std::find(m_FrontierNodes.begin(), m_FrontierNodes.end(), ReachedFrontier) == m_FrontierNodes.end())
					continue; // Since the above ReachedFrontier might NOT reach the current "frontier", continue to the next data point

				FrontierDataSize[ReachedFrontier]++;
				std::vector<int> LoneDataIdx(1, kk);
				if (AllParentNodeStatistics[ReachedFrontier].isValid())
				{
					auto NewStats = std::make_shared<S>(S(std::make_shared<DataSetIndex>(DataSetIndex(DataSetIdx->GetDataSet(), LoneDataIdx))));
					AllParentNodeStatistics[ReachedFrontier].Merge(NewStats);
				}
				else
					AllParentNodeStatistics[ReachedFrontier] = S(std::make_shared<DataSetIndex>(DataSetIndex(DataSetIdx->GetDataSet(), LoneDataIdx)));

				for (int ii = 0; ii < m_Parameters.m_NumCandidateFeatures; ++ii)
				{
					int64_t FeatureIdx = ReachedFrontier*m_Parameters.m_NumCandidateFeatures + ii;
					AllResponses[FeatureIdx].push_back(AllFeatureResponses[FeatureIdx].GetResponse(DataSetIdx->GetDataPoint(kk)));
				}
			}

			for (int jj = 0; jj < NumFrontierNodes; ++jj)
			{
				for (int ii = 0; ii < m_Parameters.m_NumCandidateFeatures; ++ii)
				{
					int64_t FeatureIdx = jj*m_Parameters.m_NumCandidateFeatures + ii;
					std::vector<VPFloat> Thresholds = SelectThresholds(AllResponses[FeatureIdx], FrontierDataSize[jj]);
					// TODO: This is wasteful, move around pointers instead
					for (int ll = 0; ll < NumCandidateThresholds; ++ll)
						AllThresholds[jj*NumSplitCandidates + ii*NumCandidateThresholds + ll] = Thresholds[ll];
				}
			}

			for (int DatItr = 0; DatItr < DataSetSize; ++DatItr) // TODO: Candidate for parallelization
			{
				int ReachedFrontier = m_Tree->TraverseToFrontier(DataSetIdx->GetDataPoint(DatItr)); // Find which node this data point reaches
				if (std::find(m_FrontierNodes.begin(), m_FrontierNodes.end(), ReachedFrontier) == m_FrontierNodes.end())
					continue; // Since the above ReachedFrontier might NOT reach the current "frontier", continue to the next data point

				for (int FeatCtr = 0; FeatCtr < m_Parameters.m_NumCandidateFeatures; ++FeatCtr)
				{
					for (int ThreshCtr = 0; ThreshCtr < NumCandidateThresholds; ++ThreshCtr)
					{
						int64_t FeatureIdx = ReachedFrontier*m_Parameters.m_NumCandidateFeatures + FeatCtr;
						VPFloat Response = AllFeatureResponses[FeatureIdx].GetResponse(DataSetIdx->GetDataPoint(DatItr));
						VPFloat Threshold = AllThresholds[ReachedFrontier*NumSplitCandidates + FeatCtr*NumCandidateThresholds + ThreshCtr];

						int64_t RowFirst3DIndex = ReachedFrontier*NumSplitCandidates + ThreshCtr*m_Parameters.m_NumCandidateFeatures + FeatCtr;
						std::vector<int> LoneDataIdx(1, DatItr);
						if (Response > Threshold) // Same logic as partitioning and testing the tree
						{
							if (AllLeftNodeStatistics[RowFirst3DIndex].isValid())
							{
								auto NewStats = std::make_shared<S>(S(std::make_shared<DataSetIndex>(DataSetIndex(DataSetIdx->GetDataSet(), LoneDataIdx))));
								AllLeftNodeStatistics[RowFirst3DIndex].Merge(NewStats);
							}
							else
								AllLeftNodeStatistics[RowFirst3DIndex] = S(std::make_shared<DataSetIndex>(DataSetIndex(DataSetIdx->GetDataSet(), LoneDataIdx)));
						}
						else
						{
							if (AllRightNodeStatistics[RowFirst3DIndex].isValid())
							{
								auto NewStats = std::make_shared<S>(S(std::make_shared<DataSetIndex>(DataSetIndex(DataSetIdx->GetDataSet(), LoneDataIdx))));
								AllRightNodeStatistics[RowFirst3DIndex].Merge(NewStats);
							}
							else
								AllRightNodeStatistics[RowFirst3DIndex] = S(std::make_shared<DataSetIndex>(DataSetIndex(DataSetIdx->GetDataSet(), LoneDataIdx)));
						}
					}
				}
			}

			// Find optimal feature responses and thresholds and stuff
			// Loop over each frontier 3D plane
			std::vector<VPFloat> OptObjVal(NumFrontierNodes, 0.0);
			std::vector<VPFloat> OptThreshold(NumFrontierNodes, 0.0);
			std::vector<T> OptFeatureResponse(NumFrontierNodes);
			for (int FrontierIdx = 0; FrontierIdx < NumFrontierNodes; ++FrontierIdx) // TODO: Candidate for parallelization
			{
				for (int FeatCtr = 0; FeatCtr < m_Parameters.m_NumCandidateFeatures; ++FeatCtr)
				{
					for (int ThreshCtr = 0; ThreshCtr < NumCandidateThresholds; ++ThreshCtr)
					{
						int64_t RowFirst3DIndex = FrontierIdx*NumSplitCandidates + ThreshCtr*m_Parameters.m_NumCandidateFeatures + FeatCtr;
						VPFloat ObjVal = GetObjectiveValue(AllParentNodeStatistics[FrontierIdx], AllLeftNodeStatistics[RowFirst3DIndex], AllRightNodeStatistics[RowFirst3DIndex]);
						if (ObjVal >= OptObjVal[FrontierIdx])
						{
							OptObjVal[FrontierIdx] = ObjVal;
							OptFeatureResponse[FrontierIdx] = AllFeatureResponses[FrontierIdx*NumSplitCandidates + FeatCtr]; // This is 2D indexed
							int64_t ColFirst3DIndex = FrontierIdx*NumSplitCandidates + FeatCtr*NumCandidateThresholds + ThreshCtr;
							OptThreshold[FrontierIdx] = AllThresholds[ColFirst3DIndex];
						}
					}
				}

				// Check for leaf creation condition
				// No gain or very small gain
				if (OptObjVal[FrontierIdx] == 0.0 || OptObjVal[FrontierIdx] < m_Parameters.m_MinGain)
				{
					m_Tree->GetNode(m_FrontierNodes[FrontierIdx]).MakeLeafNode(AllParentNodeStatistics[FrontierIdx]); // Leaf node can be "endowed" with arbitrary data. TODO: Need to handle arbitrary leaf data
					continue;
				}

				// Now free to make a split node
				m_Tree->GetNode(m_FrontierNodes[FrontierIdx]).MakeSplitNode(AllParentNodeStatistics[FrontierIdx], OptFeatureResponse[FrontierIdx], OptThreshold[FrontierIdx]);
			}
		};

		void UpdateFrontierNodes(void)
		{
			if (m_FrontierNodes.size() == 0) // First iteration, the frontier has nothing so we add the root to the frontier
			{
				m_FrontierNodes.push_back(0);
				return;
			}

			std::vector<int> LocalFrontIdx = m_FrontierNodes;
			m_FrontierNodes.clear();
			for (auto itr = LocalFrontIdx.begin(); itr != LocalFrontIdx.end(); ++itr)
			{
				int NodeIndex = (*itr);
				if (m_Tree->GetNode(NodeIndex).GetType() == Kaadugal::NodeType::Invalid)
				{
					std::cout << "[ WARN ]: Something is not right. Frontier has an invalid node which should never happen. Will NOT update frontier." << std::endl;
					m_FrontierNodes = LocalFrontIdx;
					return;
				}

				// Do nothing for leaf nodes, since this is a frontier node that would have been built already, we don't need this in the frontier
				// if(m_Tree->GetNode(NodeIndex).GetType() == Kaadugal::NodeType::LeafNode)
				// {

				// }

				if (m_Tree->GetNode(NodeIndex).GetType() == Kaadugal::NodeType::SplitNode)
				{
					// Add it's two children to the frontier
					m_FrontierNodes.push_back(2 * NodeIndex + 1); // Left child
					m_FrontierNodes.push_back(2 * NodeIndex + 2); // Right child
				}
			}
		};

		std::pair<std::shared_ptr<DataSetIndex>, std::shared_ptr<DataSetIndex>> Partition(std::shared_ptr<DataSetIndex> ParentDataSetIdx, const std::vector<VPFloat>& Responses, VPFloat Threshold) const
		{
			std::vector<int> LeftSubsetPts;
			std::vector<int> RightSubsetPts;
			for (int i = 0; i < ParentDataSetIdx->Size(); ++i)
			{
				if (Responses[i] > Threshold) // Please use same logic when testing the tree
					LeftSubsetPts.push_back(ParentDataSetIdx->GetDataPointIndex(i));
				else
					RightSubsetPts.push_back(ParentDataSetIdx->GetDataPointIndex(i));
			}

			return std::make_pair(std::shared_ptr<DataSetIndex>(new DataSetIndex(ParentDataSetIdx->GetDataSet(), LeftSubsetPts))
				, std::shared_ptr<DataSetIndex>(new DataSetIndex(ParentDataSetIdx->GetDataSet(), RightSubsetPts)));
		};

		const std::vector<VPFloat> SelectThresholds(const std::vector<VPFloat>& Responses, const int DataSubsetSize)
		{
			// std::cout << "Dataset Size: " << DataSubsetIdx->Size() << std::endl;
			// std::cout << "Responses Size: " << Responses.size() << std::endl;
			// Please see Efficient Implementation of Decision Forests, Shotton et al. 2013
			// Section 21.3.3 explains how to implement this threshold selection using quantiles
			// Also see the Sherwood Library from Microsoft Research
			std::vector<VPFloat> Thresholds(m_Parameters.m_NumCandidateThresholds); // This is different from the Sherwood implementation. We don't use n+1
			std::vector<VPFloat> Quantiles(m_Parameters.m_NumCandidateThresholds + 1); // TODO: Candidate for memory saving

			// This isn't ideal because if size of data subset is only a few above NumThresh, then Randomizer will repeat some values
			if (DataSubsetSize > m_Parameters.m_NumCandidateThresholds)
			{
				// Sample m_NumCandidateThresholds+1 times (uniformly randomly) from Responses
				std::uniform_int_distribution<int> UniDist(0, int(DataSubsetSize - 1)); // Both inclusive
				for (int i = 0; i < m_Parameters.m_NumCandidateThresholds + 1; i++)
					Quantiles[i] = Responses[UniDist(Randomizer::Get().GetRNG())];
			}
			else
			{
				Quantiles.resize(Responses.size());
				Thresholds.resize(Responses.size() - 1); // One less than quantiles
				std::copy(Responses.begin(), Responses.end(), Quantiles.begin());
			}

			// Now compute quantiles. See https://www.stat.auckland.ac.nz/~ihaka/787/lectures-quantiles-handouts.pdf
			// if you don't know how to do this
			std::sort(Quantiles.begin(), Quantiles.end());

			if (Quantiles[0] == Quantiles[Quantiles.size() - 1])
				return std::vector<VPFloat>(); // Looks like samples were all the same. This is bad


			// Compute n candidate thresholds by sampling in between n+1 approximate quantiles
			std::uniform_real_distribution<VPFloat> UniRealDist(0, 1); // [0, 1), NOTE the exclusive end
			int NumThresholds = Thresholds.size();
			for (int i = 0; i < NumThresholds; ++i)
				Thresholds[i] = Quantiles[i] + VPFloat(UniRealDist(Randomizer::Get().GetRNG()) * (Quantiles[i + 1] - Quantiles[i]));

			// std::cout << "Before return:\n";
			// for(int j = 0; j < Thresholds.size(); ++j)
			// 	std::cout << "Thresh: " << Thresholds[j] << std::endl;

			return Thresholds;
		};

		VPFloat GetObjectiveValue(S& ParentStats, S& LeftStats, S& RightStats)
		{
			if (ParentStats.GetNumDataPoints() < m_Parameters.m_MinDataSetSize)
			{
				std::cout << "RUNTIME ERROR in GetObjectiveValue()" << std::endl; // For windows
				throw std::runtime_error("ParentStats should never contain so little data points. Cannot proceed with training.");
			}

			// If there are fewer than requested datapoints in this split, we assign a value that shows that we do not prefer this split
			if (LeftStats.GetNumDataPoints() <  std::max(3, m_Parameters.m_MinDataSetSize)
				|| RightStats.GetNumDataPoints() <  std::max(3, m_Parameters.m_MinDataSetSize))
				return 0.0;

			// Assuming statistics are already aggregated
			// NOTE: We are using information gain as the objective function
			// Change this and use a template/abstract class, if needed
			// See any of the Shotton et al. papers for this definition
			VPFloat InformationGain = ParentStats.GetEntropy()
				- (VPFloat(LeftStats.GetNumDataPoints())  * LeftStats.GetEntropy()
				+ VPFloat(RightStats.GetNumDataPoints()) * RightStats.GetEntropy()) / VPFloat(ParentStats.GetNumDataPoints());

			//std::cout << "InfoGain: " << InformationGain << std::endl;

			return InformationGain;
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

