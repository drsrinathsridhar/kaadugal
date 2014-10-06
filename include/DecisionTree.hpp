#ifndef _DECISIONTREE_HPP_
#define _DECISIONTREE_HPP_

#include <vector>
#include <stdexcept>

#include "DecisionNode.hpp"

namespace Kaadugal
{
    // T: AbstractFeatureResponse which is the feature response function or weak learner
    // S: AbstractStatistics which contains some statistics about node from training
    // R: AbstractLeafData, arbitrary data stored if this is a leaf node
    template<class T, class S, class R>
    class DecisionTree
    {
    private:
	std::vector<DecisionNode<T, S, R>> m_Nodes;
	int m_MaxDecisionLevels; // The root node is level 0

    public:
	DecisionTree(int MaxDecisionLevels)
	    : m_MaxDecisionLevels(MaxDecisionLevels)
	{
	    if(MaxDecisionLevels < 0)
	    {
		std::cout << "[ WARN ]: Passed negative value for MaxDecisionLevels. Setting to default of 10." << std::endl;
		MaxDecisionLevels = 10;
		m_MaxDecisionLevels = MaxDecisionLevels;
	    }

	    // Following advice from Efficient Implementation of Decision Forests, Shotton et al. 2013
	    // While allocating for more nodes than needed (in case trees are unbalanced which is bad anyway)
	    // is wasteful memory-wise, it is efficient for access during testing.

	    // NOTE: Will crash if this exceeds available system memory
	    m_Nodes.resize( (1 << (MaxDecisionLevels + 1)) - 1 ); // 2^(l+1) - 1
	};

	const std::vector<DecisionNode<T, S, R>>& GetAllNodes(void) { return m_Nodes; };
	const DecisionNode<T, S, R>& GetNode(int i) const { return m_Nodes[i]; }; // Read-only
	DecisionNode<T, S, R>& GetNode(int i) { return m_Nodes[i]; };
	const int GetNumNodes(void) { return m_Nodes.size(); };
	const int GetMaxDecisionLevels(void) { return m_MaxDecisionLevels; };

	bool isValid(void)
	{
	    if(GetNumNodes() <= 0)
		return false;
	    if(GetNode(0).GetType() == Kaadugal::Invalid)
		return false;

	    return true;
	};

	// Stream write methods
	virtual void Serialize(std::ostream& Out) const
	{
	    // TODO:
	};
	virtual void Deserialize(std::istream& In)
	{
	    // TODO:
	};

	// Render methods for visualizing node
	virtual void Render(void)
	{
	    // TODO:
	};
    };
} // namespace Kaadugal

#endif // _DECISIONTREE_HPP_
