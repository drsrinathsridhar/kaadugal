#ifndef _DECISIONFORESTBUILDER_HPP_
#define _DECISIONFORESTBUILDER_HPP_

#include <memory>

#include "DecisionForest.hpp"
#include "DecisionTreeBuilder.hpp"

namespace Kaadugal
{
    // T: AbstractFeatureResponse which is the feature response function or weak learner
    // S: AbstractStatistics which contains some statistics about node from training
    // R: AbstractLeafData, arbitrary data stored if this is a leaf node
    template<class T, class S, class R>
    class DecisionForestBuilder
    {
    private:
	AbstractDataSet m_DataSet; // Dataset should never be modified
	const ForestBuilderParameters& m_Parameters; // Parameters also should never be modified
	std::vector<DecisionTreeBuilder<T,S,R>> m_TreeBuilders;
	DecisionForest<T,S,R> m_Forest;
	bool m_isForestTrained;

    public:
	DecisionForestBuilder(const ForestBuilderParameters& Parameters)
	    : m_Parameters(Parameters)
	    , m_isForestTrained(false)
	{
	    for(int i = 0; i < m_Parameters.m_NumTrees; ++i)
	    {
		m_TreeBuilders.push_back(DecisionTreeBuilder<T,S,R>(m_Parameters));
	    }
	};

	bool Build(const AbstractDataSet& DataSet)
	{
	    m_DataSet = DataSet;
	    bool Success = true;
	    // TODO: Randomly partition data set
	    for(int i = 0; i < m_TreeBuilders.size(); ++i)
	    {
		Success &= m_TreeBuilders[i].Build(m_DataSet);
		m_Forest.AddTree(m_TreeBuilders[i].GetTree());
	    }

	    m_isForestTrained = Success;
	    return m_isForestTrained;
	};	

	const DecisionForest<T,S,R>& GetForest(void) { return m_Forest; };
	const bool DoneBuild(void) { return m_isForestTrained; };
    };
} // namespace Kaadugal

#endif // _DECISIONFORESTBUILDER_HPP_
