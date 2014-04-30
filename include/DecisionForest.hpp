#ifndef _DECISIONFOREST_HPP_
#define _DECISIONFOREST_HPP_

#include "AbstractFeatureResponse.hpp"
#include "AbstractStatistics.hpp"
#include "AbstractLeafData.hpp"
#include "DecisionTree.hpp"

namespace Kaadugal
{
    // T: AbstractFeatureResponse which is the feature response function or weak learner
    // S: AbstractStatistics which contains some statistics about node from training
    // R: AbstractLeafData, arbitrary data stored if this is a leaf node
    template<class T, class S, class R>
    class DecisionForest
    {
	
    };
} // namespace Kaadugal

#endif // _DECISIONFOREST_HPP_
