#ifndef _DECISIONFOREST_HPP_
#define _DECISIONFOREST_HPP_

#include <memory>

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

	void Test(std::shared_ptr<AbstractDataPoint> DataPointPtr, std::shared_ptr<S> ForestLeafStats)
	{
	    // TODO: Handle arbitrary leaf data
	    for(int i = 0; i < m_nTrees; ++i)
		ForestLeafStats->Merge(m_Trees[i]->Test(DataPointPtr));
	};
    };
} // namespace Kaadugal

#endif // _DECISIONFOREST_HPP_
