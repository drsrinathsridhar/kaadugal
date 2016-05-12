#pragma once

#include <vector>
#include <stdexcept>

#include "DecisionNode.hpp"
#include "Abstract/AbstractDataSet.hpp"

namespace Kaadugal
{
	// T: AbstractFeatureResponse which is the feature response function or weak learner
	// S: AbstractStatistics which contains some statistics about node from training
	// R: AbstractLeafData, arbitrary data stored if this is a leaf node
	template<class T, class S, class R>
	class DecisionTree
	{
		// TODO: Many methods need to be made const
	private:
		std::vector<DecisionNode<T, S, R>> m_Nodes;
		int m_MaxDecisionLevels; // The root node is level 0
		int m_NumNodes;

		int TraverseRecursive(const std::shared_ptr<AbstractDataPoint>& DataPointPtr, int NodeIndex)
		{
			int OutputNodeIndex = NodeIndex;
			if (m_Nodes[NodeIndex].GetType() == Kaadugal::NodeType::LeafNode
				|| m_Nodes[NodeIndex].GetType() == Kaadugal::NodeType::Invalid) // Termination condition
			{
				return OutputNodeIndex;
			}

			if (m_Nodes[NodeIndex].GetFeatureResponse().GetResponse(DataPointPtr) > m_Nodes[NodeIndex].GetThreshold()) // Go left. This is same logic as in Tree builder, partition
				OutputNodeIndex = TraverseRecursive(DataPointPtr, 2 * NodeIndex + 1);
			else // Go right
				OutputNodeIndex = TraverseRecursive(DataPointPtr, 2 * NodeIndex + 2);

			return OutputNodeIndex;
		};


	public:
		DecisionTree(int MaxDecisionLevels = 0)
			: m_MaxDecisionLevels(MaxDecisionLevels)
		{
			if (MaxDecisionLevels < 0)
			{
				std::cout << "[ WARN ]: Passed negative value for MaxDecisionLevels. Setting to default of 10." << std::endl;
				MaxDecisionLevels = 10;
				m_MaxDecisionLevels = MaxDecisionLevels;
			}

			// Following advice from Efficient Implementation of Decision Forests, Shotton et al. 2013
			// While allocating for more nodes than needed (in case trees are unbalanced which is bad anyway)
			// is wasteful memory-wise, it is efficient for access during testing.

			// NOTE: Will crash if this exceeds available system memory
			m_Nodes.resize((1 << (MaxDecisionLevels + 1)) - 1); // 2^(l+1) - 1
			m_NumNodes = m_Nodes.size();
		};

		void Serialize(std::ostream& OutputStream) const
		{
			OutputStream.write((const char *)(&m_MaxDecisionLevels), sizeof(int));
			OutputStream.write((const char *)(&m_NumNodes), sizeof(int));
			for (int i = 0; i < m_NumNodes; ++i)
				m_Nodes[i].Serialize(OutputStream);
		};

		void Deserialize(std::istream& InputStream)
		{
			InputStream.read((char *)(&m_MaxDecisionLevels), sizeof(int));
			InputStream.read((char *)(&m_NumNodes), sizeof(int));
			// NOTE: Will crash if this exceeds available system memory
			int nLeaves = 0;
			m_Nodes.resize(m_NumNodes);
			for (int i = 0; i < m_NumNodes; ++i)
			{
				m_Nodes[i].Deserialize(InputStream);
				if (m_Nodes[i].GetType() == Kaadugal::LeafNode)
					nLeaves++;
			}
			//std::cout << "Number of leaves: " << nLeaves << std::endl;
		};

		const std::vector<DecisionNode<T, S, R>>& GetAllNodes(void) { return m_Nodes; };
		const DecisionNode<T, S, R>& GetNode(int i) const { return m_Nodes[i]; }; // Read-only
		DecisionNode<T, S, R>& GetNode(int i) { return m_Nodes[i]; };
		const int& GetNumNodes(void) { return m_NumNodes; };
		const int& GetMaxDecisionLevels(void) { return m_MaxDecisionLevels; };

		const std::shared_ptr<S> Test(std::shared_ptr<AbstractDataPoint> DataPointPtr, std::shared_ptr<R> LeafData = nullptr)
		{
			if (isValid() == false)
				std::cout << "[ WARN ]: This tree is invalid. Cannot test data point." << std::endl;

			// Stats
			S TreeLeafStats;
			int LeafNodeIdx = TestRecursive(DataPointPtr, 0, TreeLeafStats);
			
			// Leaf data
			if (LeafData != nullptr)
			{
				R TreeLeafData = m_Nodes[LeafNodeIdx].GetLeafData();
				LeafData->Merge(std::make_shared<R>(TreeLeafData));
			}

			return std::make_shared<S>(TreeLeafStats);
		};

		int TestRecursive(const std::shared_ptr<AbstractDataPoint>& DataPointPtr, int NodeIndex, S& TreeLeafStats)
		{
			if (m_Nodes[NodeIndex].GetType() == Kaadugal::NodeType::LeafNode) // Termination condition
			{
				//std::cout << "Terminating NodeIndex: " << NodeIndex << std::endl;
				TreeLeafStats = m_Nodes[NodeIndex].GetStatistics();

				return NodeIndex;
			}

			// std::cout << "Response: " << m_Nodes[NodeIndex].GetFeatureResponse().GetResponse(DataPointPtr) << std::endl;
			// std::cout << "Threshold: " << m_Nodes[NodeIndex].GetThreshold() << std::endl << std::endl;
			if (m_Nodes[NodeIndex].GetFeatureResponse().GetResponse(DataPointPtr) > m_Nodes[NodeIndex].GetThreshold()) // Go left. This is same logic as in Tree builder, partition
				return TestRecursive(DataPointPtr, 2 * NodeIndex + 1, TreeLeafStats);

				return TestRecursive(DataPointPtr, 2 * NodeIndex + 2, TreeLeafStats);
		};

		// Return the node index for the leaf or the first invalid node reached by the data point
		int TraverseToFrontier(std::shared_ptr<AbstractDataPoint> DataPointPtr)
		{
			if (GetNumNodes() <= 0)
				std::cout << "[ WARN ]: No nodes in this tree. Check config." << std::endl;

			int FrontierIndex = TraverseRecursive(DataPointPtr, 0);

			return FrontierIndex;
		};

		bool isValid(void)
		{
			if (GetNumNodes() <= 0)
				return false;
			if (GetNode(0).GetType() == Kaadugal::Invalid)
				return false;

			return true;
		};

		// Render methods for visualizing node
		virtual void Render(void)
		{
			// TODO:
		};
	};
} // namespace Kaadugal
