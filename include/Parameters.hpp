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
	VPFloat m_MinGain; // Minimum gain to tolerate
	
	ForestBuilderParameters(int NumTrees, int MaxLevels, int NumCandidateFeatures, int NumCandidateThresholds, VPFloat MinGain, TrainMethod Type = TrainMethod::DFS)
	    : m_NumTrees(NumTrees)
	    , m_MaxLevels(MaxLevels)
	    , m_NumCandidateFeatures(NumCandidateFeatures)
	    , m_NumCandidateThresholds(NumCandidateThresholds)
	    , m_TrainMethod(Type)
	    , m_MinGain(MinGain)
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
	    m_MinGain = RHS.m_MinGain;

	    return *this;
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
		int ConfigCtr = 0;
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
				ConfigCtr++;
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

				ConfigCtr++;
			    }
			    if(Key == "MaxTreeLevels")
			    {
				m_MaxLevels = std::atoi(Value.c_str());
				ConfigCtr++;
			    }
			    if(Key == "NumCandidateFeats")
			    {
				m_NumCandidateFeatures = std::atoi(Value.c_str());
				ConfigCtr++;
			    }
			    if(Key == "NumCandidateThresh")
			    {
				m_NumCandidateThresholds = std::atoi(Value.c_str());
				ConfigCtr++;
			    }
			    if(Key == "MinGain")
			    {
				m_MinGain = VPFloat(std::atof(Value.c_str()));
				ConfigCtr++;
			    }

			    isKey = false;
			    continue;
			}
		    }
		}

		if(ConfigCtr < 6) // Greater than 6 is fine because we can have other parameters in the config for client code
		{
		    std::cout << "[ WARN ]: Some parameters are missing (read only " << ConfigCtr << "). Please check input." << std::endl;
		    m_isValid = false;
		    return;
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
