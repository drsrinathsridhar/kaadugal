#ifndef _DECISIONTREEBUILDER_HPP_
#define _DECISIONTREEBUILDER_HPP_

#include <memory>
#include <fstream>
#include <sstream>
#include <string>

#include "DecisionTree.hpp"
#include "AbstractDataSet.hpp"

namespace Kaadugal
{

    enum TreeBuildType
    {
	DFS,
	BFS,
	Hybrid
    };
    
    class TreeBuilderParameters
    {
    public:
	int m_MaxLevels;
	int m_NumCandidateThresholds;
	TreeBuildType m_Type;
	bool m_isValid;
	
	TreeBuilderParameters(int MaxLevels, int NumCandidateThresholds, TreeBuildType Type = TreeBuildType::DFS)
	    : m_MaxLevels(MaxLevels)
	    , m_NumCandidateThresholds(NumCandidateThresholds)
	    , m_Type(Type)
	{

	};

	TreeBuilderParameters(const std::string& ParameterFile)
	{
	    Deserialize(ParameterFile);
	};

	void Serialize(const std::string& ParameterFile)
	{
	    // TODO:
	};

	void Deserialize(const std::string& ParameterFile)
	{
	    std::fstream ParamFile(ParameterFile, std::ios::in);
	    if(ParamFile.is_open())
	    {
		// Read parameters from file
		m_isValid = true;
		std::string Line;
		int ConfigCtr = 0; // Counts how many of the minimum configurations are present. Throw error otherwise.
		while(std::getline(ParamFile, Line))
		{
		    // Skip empty lines or lines beginning with #
		    if(Line.size() < 1)
			continue;
		    if(Line.data()[0] == '#') // Comment lines
			continue;

		    std::stringstream LineStream(Line);
		    std::string Token, Key, Value;
		    bool isKey = false;
		    while(std::getline(LineStream, Token, ':'))
		    {
			Token.erase(0, Token.find_first_not_of(" \t")); // Trim to remove leading spaces
			if(isKey == false)
			{
			    isKey = true;
			    Key = Token;
			    continue;
			}
			else
			{
			    Value = Token;
			    if(Key == "Type")
			    {
				if(Value == "BFS")
				    m_Type = TreeBuildType::BFS;
				if(Value == "DFS")
				    m_Type = TreeBuildType::DFS;
				if(Value == "Hybrid")
				    m_Type = TreeBuildType::Hybrid;
				// std::cout << Value << std::endl;
			    }
			    if(Key == "MaxTreeLevels")
			    {
				m_MaxLevels = std::atoi(Value.c_str());
				// std::cout << m_MaxLevels << std::endl;
			    }
			    if(Key == "NumCandidateThresh")
			    {
				m_NumCandidateThresholds = std::atoi(Value.c_str());
				// std::cout << m_NumCandidateThresholds << std::endl;
			    }
			    isKey = false;
			    continue;
			}
		    }
		}

		// Print info for debug purposes
		std::cout << "[ INFO ]: Creating forest with trees with a depth of upto " << m_MaxLevels
			  << " levels, trained using method " << m_Type << ", and "
			  << m_NumCandidateThresholds << " candidate thresholds." << std::endl;
	    }
	    else
	    {
		std::cout << "[ WARN ]: Unable to open parameters file. Please check input." << std::endl;
		m_isValid = false;
            }
	};		
    };

    // T: AbstractFeatureResponse which is the feature response function or weak learner
    // S: AbstractStatistics which contains some statistics about node from training
    // R: AbstractLeafData, arbitrary data stored if this is a leaf node
    template<class T, class S, class R>
    class DecisionTreeBuilder
    {
    private:
	std::unique_ptr<DecisionTree<T, S, R>> m_Tree;
	const AbstractDataSet& m_DataSet; // Dataset should never be modified
	const TreeBuilderParameters& m_Parameters; // Parameters also should never be modified

    public:
	DecisionTreeBuilder(const TreeBuilderParameters& Parameters)
	    : m_Parameters(Parameters)
	{
	    m_Tree = new DecisionTree<T, S, R>(Parameters.m_MaxLevels);
	};

	void BuildTree(const AbstractDataSet& DataSet)
	{
	    m_DataSet = DataSet;
	    if(m_Parameters.m_Type == TreeBuildType::DFS)
		BuildTreeDepthFirst(DataSet, 0, 0);
	    if(m_Parameters.m_Type == TreeBuildType::BFS)
		BuildTreeBreadthFirst();
	    if(m_Parameters.m_Type == TreeBuildType::Hybrid)
		BuildTreeHybrid();
	}

	void BuildTreeDepthFirst(const AbstractDataSet& PartitionedDataSet, int NodeIndex, int CurrentNodeDepth)
	{
	    if(CurrentNodeDepth > m_Tree.GetMaxDecisionLevels())
	    {
		m_Tree.GetNode(NodeIndex).MakeLeafNode();
		return;
	    }

	    // Initialize optimal values
	    VPFloat OptGain = -std::numeric_limits<VPFloat>::infinity();
	    T OptFeatureResponse; // This should create empty feature response
	    
	    for(int i = 0; i < m_Parameters.m_NumCandidateThresholds; ++i)
	    {
		T FeatureResponse(1); // This should sample from possible elements in the feature response set
	    };	    
	};

	void BuildTreeBreadthFirst(void)
	{
	    // TODO:
	};

	void BuildTreeHybrid(void)
	{
	    // TODO:	    
	};
    };
} // namespace Kaadugal

#endif // _DECISIONTREEBUILDER_HPP_
