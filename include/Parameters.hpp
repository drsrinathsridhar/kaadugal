#ifndef _PARAMETERS_HPP_
#define _PARAMETERS_HPP_

#include <memory>
#include <fstream>
#include <sstream>
#include <string>

namespace Kaadugal
{
    enum TrainMethod
    {
	DFS,
	BFS,
	Hybrid
    };
    
    class ForestBuilderParameters
    {
    public:
	int m_NumTrees;
	int m_MaxLevels;
	int m_NumCandidateFeatures;
	int m_NumCandidateThresholds;
	TrainMethod m_TrainMethod;
	bool m_isValid;
	
	ForestBuilderParameters(int NumTrees, int MaxLevels, int NumCandidateFeatures, int NumCandidateThresholds, TrainMethod Type = TrainMethod::DFS)
	    : m_NumTrees(NumTrees)
	    , m_MaxLevels(MaxLevels)
	    , m_NumCandidateFeatures(NumCandidateFeatures)
	    , m_NumCandidateThresholds(NumCandidateThresholds)
	    , m_TrainMethod(Type)
	{

	};

	ForestBuilderParameters(const std::string& ParameterFile)
	{
	    Deserialize(ParameterFile);
	};

	ForestBuilderParameters& operator=(const ForestBuilderParameters& RHS)
	{
	    m_NumTrees = RHS.m_NumTrees;
	    m_MaxLevels = RHS.m_MaxLevels;
	    m_NumCandidateFeatures = RHS.m_NumCandidateFeatures;
	    m_NumCandidateThresholds = RHS.m_NumCandidateThresholds;
	    m_TrainMethod = RHS.m_TrainMethod;
	    m_isValid = RHS.m_isValid;
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
			    if(Key == "NumTrees")
			    {
				m_NumTrees = std::atoi(Value.c_str());
				// std::cout << m_NumTrees << std::endl;
			    }			
			    if(Key == "TrainMethod")
			    {
				if(Value == "BFS")
				    m_TrainMethod = TrainMethod::BFS;
				if(Value == "DFS")
				    m_TrainMethod = TrainMethod::DFS;
				if(Value == "Hybrid")
				    m_TrainMethod = TrainMethod::Hybrid;
			    }
			    if(Key == "MaxTreeLevels")
				m_MaxLevels = std::atoi(Value.c_str());
			    if(Key == "NumCandidateFeats")
				m_NumCandidateFeatures = std::atoi(Value.c_str());
			    if(Key == "NumCandidateThresh")
				m_NumCandidateThresholds = std::atoi(Value.c_str());
			    isKey = false;
			    continue;
			}
		    }
		}

		// Print info for debug purposes
		std::cout << "[ INFO ]: Creating forest with trees with a depth of upto " << m_MaxLevels
			  << " levels, trained using method " << m_TrainMethod << ", and "
			  << m_NumCandidateThresholds << " candidate thresholds." << std::endl;
	    }
	    else
	    {
		std::cout << "[ WARN ]: Unable to open parameters file. Please check input." << std::endl;
		m_isValid = false;
            }
	};		
    };
} // namespace Kaadugal

#endif // _PARAMETERS_HPP_
