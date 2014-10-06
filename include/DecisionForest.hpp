#ifndef _DECISIONFOREST_HPP_
#define _DECISIONFOREST_HPP_

#include <memory>

#include "DecisionTree.hpp"

namespace Kaadugal
{
    // T: AbstractFeatureResponse which is the feature response function or weak learner
    // S: AbstractStatistics which contains some statistics about node from training
    // R: AbstractLeafData, arbitrary data stored if this is a leaf node
    template<class T, class S, class R>
    class DecisionForest
    {
    private:
	std::vector<std::shared_ptr<DecisionTree<T, S, R>>> m_Trees;

    public:
	void AddTree(std::shared_ptr<DecisionTree<T, S, R>> TreePtr) { m_Trees.push_back(TreePtr); };
    };
} // namespace Kaadugal

#endif // _DECISIONFOREST_HPP_
