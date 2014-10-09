#include <iostream>

#include "Kaadugal.hpp"
#include "DecisionForestBuilder.hpp"
#include "PointSet2D.hpp"

std::string g_ParamFileName;
std::string g_DataFileName;
bool g_isTrainMode;

void PrintUsage(char * argv[])
{
    std::cout << "[ USAGE ]: " << argv[0] << " (train|test) <CONFIG_FILE_PATH> <DATA_FILE>" << std::endl;
}

bool ParseArguments(int argc, char * argv[])
{
    if(argc != 4)
    {
	PrintUsage(argv);
	return false;
    }
    if(std::string(argv[1]) != "train" && std::string(argv[1]) != "test")
    {
	PrintUsage(argv);
	return false;
    }
    g_ParamFileName = argv[2];
    g_DataFileName = argv[3];
    if(std::string(argv[1]) == "train")
	g_isTrainMode = true;
    else
	g_isTrainMode = false;	

    return true;
}

int main(int argc, char * argv[])
{
    if(ParseArguments(argc, argv) == false)
	return -1;

    if(g_isTrainMode)
    {
	// Load parameters from file
	Kaadugal::ForestBuilderParameters ForestParams(g_ParamFileName);
	PointSet2D Point2DData(g_DataFileName);
	std::shared_ptr<Kaadugal::AbstractDataSet> Point2DDataPtr = std::make_shared<PointSet2D>(Point2DData);

	// Build forest from training data
	Kaadugal::DecisionForestBuilder<Kaadugal::AbstractFeatureResponse
					, Kaadugal::AbstractStatistics
					, Kaadugal::AbstractLeafData> ForestBuilder(ForestParams);
	if(ForestBuilder.Build(Point2DDataPtr) == true)
	{
	    std::cout << "Random Forest successfully trained." << std::endl;
	    int t;
	    std::cin >> t;
	}
	else
	    std::cout << "[ ERROR ]: Unable to train forest." << std::endl;
    }
    else
	std::cout << "[ WARN ]: Tree testing not implemented yet." << std::endl;

    return 0;
}
