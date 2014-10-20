#include <iostream>
#include <fstream>

#include "Kaadugal.hpp"
#include "DecisionForestBuilder.hpp"
#include "PointSet2D.hpp"
#include "AAFeatureResponse2D.hpp"
#include "HistogramStats.hpp"

bool g_isTrainMode;
std::string g_DataFileName;
PointSet2D g_Point2DData;

// Training members
std::string g_ParamFileName;
std::string g_OutputForestFName;

// Testing members
std::string g_InputForestFName;

void PrintUsage(char * argv[])
{
    std::cout << "[ USAGE ]: " << argv[0] << " (train <CONFIG_FILE_PATH> <OUTPUT_FOREST_PATH> | test <INPUT_FOREST_PATH>)  <DATA_FILE>" << std::endl;
}

bool ParseArguments(int argc, char * argv[])
{
    if(argc < 4 || argc > 5)
    {
	PrintUsage(argv);
	return false;
    }
    if(std::string(argv[1]) != "train" && std::string(argv[1]) != "test")
    {
	PrintUsage(argv);
	return false;
    }

    if(std::string(argv[1]) == "train" && argc == 5)
    {
	g_ParamFileName = argv[2];
	g_OutputForestFName = argv[3];
	g_DataFileName = argv[4];
	g_isTrainMode = true;
	return true;
    }
    if(std::string(argv[1]) == "test" && argc == 4)
    {
	g_InputForestFName = argv[2];
	g_DataFileName = argv[3];
	g_isTrainMode = false;
	return true;
    }
    PrintUsage(argv);
    return false;
}

int Train(void)
{
    // Load parameters from file
    Kaadugal::ForestBuilderParameters ForestParams(g_ParamFileName);
    std::shared_ptr<Kaadugal::AbstractDataSet> Point2DDataPtr = std::make_shared<PointSet2D>(g_Point2DData);

    // Build forest from training data
    Kaadugal::DecisionForestBuilder<AAFeatureResponse2D, HistogramStats> ForestBuilder(ForestParams);
    if(ForestBuilder.Build(Point2DDataPtr) == true)
    {
	std::cout << "Random Forest successfully trained." << std::endl;

	std::cout << "Writing forest to file..." << std::endl;
	std::filebuf FileBuf;
	FileBuf.open(g_OutputForestFName, std::ios::out | std::ios::trunc | std::ios::binary);
	if(FileBuf.is_open())
	{
	    std::ostream OutForest(&FileBuf);
	    ForestBuilder.GetForest().Serialize(OutForest);
	    FileBuf.close();
	    std::cout << "Done." << std::endl;
	}
	else
	    std::cout << "[ WARN ]: Unable to open file to save." << std::endl;
    }
    else
	std::cout << "[ ERROR ]: Unable to train forest." << std::endl;

    return 0;
}

int Test(void)
{
    Kaadugal::DecisionForest<AAFeatureResponse2D, HistogramStats, Kaadugal::AbstractLeafData> LoadedForest;
    std::cout << "Loading forest from file..." << std::endl;
    std::filebuf FileBuf;
    FileBuf.open(g_InputForestFName, std::ios::in | std::ios::binary);
    if(FileBuf.is_open())
    {
	std::istream InForest(&FileBuf);
	LoadedForest.Deserialize(InForest);
	FileBuf.close();
	std::cout << "Done." << std::endl;
    }
    else
    {
	std::cout << "[ WARN ]: Unable to open forest file. Exiting." << std::endl;
	return -2;
    }
		
    std::cout << "Now testing trained forest with data..." << std::endl;
    int SuccessCtr = 0;
    int DataSize = g_Point2DData.Size();
    for(int i = 0; i < DataSize; ++i)
    {
	std::shared_ptr<Kaadugal::AbstractDataPoint> TestPointPtr = std::dynamic_pointer_cast<Kaadugal::AbstractDataPoint>(g_Point2DData.Get(i));
	std::shared_ptr<HistogramStats> FinalStatsPtr = std::make_shared<HistogramStats>(HistogramStats(4));
	LoadedForest.Test(TestPointPtr, FinalStatsPtr);
	// std::cout << "Winner: " << FinalStatsPtr->FindWinnerLabelIndex() << std::endl;
	// std::cout << "Actual: " << std::dynamic_pointer_cast<Point2D>(g_Point2DData.Get(DataPtNum))->GetLabel() << std::endl;
	if(FinalStatsPtr->FindWinnerLabelIndex() == std::dynamic_pointer_cast<Point2D>(g_Point2DData.Get(i))->GetLabel())
	    SuccessCtr++;
    }
    std::cout << "Classification Accuracy: " << float(SuccessCtr) / float(DataSize) * 100.0 << std::endl;

    return 0;
}

int main(int argc, char * argv[])
{
    if(ParseArguments(argc, argv) == false)
	return -1;

    g_Point2DData = PointSet2D(g_DataFileName);
    if(g_isTrainMode)
	return Train();
    else
	return Test();

    return 0;
}
