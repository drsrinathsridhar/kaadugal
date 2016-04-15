#ifndef _DECISIONFOREST_HPP_
#define _DECISIONFOREST_HPP_

#include <memory>
#include <ostream>
#include <istream>

#include "DecisionTree.hpp"
#include "Abstract/AbstractDataSet.hpp"
#include "Abstract/AbstractStatistics.hpp"

namespace Kaadugal
{
	// T: AbstractFeatureResponse which is the feature response function or weak learner
	// S: AbstractStatistics which contains some statistics about node from training
	// R: AbstractLeafData, arbitrary data stored if this is a leaf node
	template<class T, class S, class R>
	class DecisionForest
	{
	private:
		int m_nTrees;
		std::vector<std::shared_ptr<DecisionTree<T, S, R>>> m_Trees;

	public:
		void AddTree(std::shared_ptr<DecisionTree<T, S, R>> TreePtr)
		{
			m_Trees.push_back(TreePtr);
			m_nTrees = m_Trees.size();
		};

		int GetNumTrees(void) { return m_nTrees; };

		void Test(std::shared_ptr<AbstractDataPoint> DataPointPtr, std::shared_ptr<S> ForestLeafStats, std::shared_ptr<R> LeafData = nullptr)
		{
			for (int i = 0; i < m_nTrees; ++i)
			{
				if (LeafData != nullptr)
				{
					std::shared_ptr<R> TreeLeafData = std::make_shared<R>();
					auto Stats = m_Trees[i]->Test(DataPointPtr, TreeLeafData);

					ForestLeafStats->Merge(Stats);
					LeafData->Merge(TreeLeafData);
				}
				else
					ForestLeafStats->Merge(m_Trees[i]->Test(DataPointPtr));
			}
		};

		void Serialize(std::ostream& OutputStream) const
		{
			OutputStream.write((const char *)(&m_nTrees), sizeof(int));
			for (int i = 0; i < m_nTrees; ++i)
				m_Trees[i]->Serialize(OutputStream);
		};
		void Deserialize(std::istream& InputStream)
		{
			int nTrees = 0;
			InputStream.read((char *)(&nTrees), sizeof(int));
			for (int i = 0; i < nTrees; ++i)
			{
				auto Tree = std::shared_ptr<DecisionTree<T, S, R>>(new DecisionTree<T, S, R>(0)); // Create empty tree
				Tree->Deserialize(InputStream);
				AddTree(Tree);
			}
		};
	};
} // namespace Kaadugal

#endif // _DECISIONFOREST_HPP_
