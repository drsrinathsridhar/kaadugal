#include <iostream>

#include "Kaadugal.hpp"
#include "DecisionForestBuilder.hpp"
#include "PointSet2D.hpp"
#include "AAFeatureResponse2D.hpp"
#include "HistogramStats.hpp"

std::string g_ParamFileName;
std::string g_DataFileName;
bool g_isTrainMode;
bool g_isSuccessTrained = false;

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
	Kaadugal::DecisionForestBuilder<AAFeatureResponse2D, HistogramStats> ForestBuilder(ForestParams);
	if(ForestBuilder.Build(Point2DDataPtr) == true)
	{
	    g_isSuccessTrained = true;
	    std::cout << "Random Forest successfully trained." << std::endl;

	    // Now test
	    if(g_isSuccessTrained)
	    {
		std::cout << "Now testing trained forest with sample data..." << std::endl;
		int SuccessCtr = 0;
		int Total = 100;
		for(int i = 0; i < Total; ++i)
		{
		    std::uniform_int_distribution<int> UniDist(0, Point2DData.Size() - 1); // Both inclusive
		    int DataPtNum = UniDist(Kaadugal::Randomizer::Get().GetRNG());
		    std::uniform_real_distribution<Kaadugal::VPFloat> UniRealDist(0, 150.0); // [0, 10)
		    std::cout << "Test point (before perturb): " << std::dynamic_pointer_cast<Point2D>(Point2DData.Get(DataPtNum))->m_x << ", " << std::dynamic_pointer_cast<Point2D>(Point2DData.Get(DataPtNum))->m_y << std::endl;
		    std::dynamic_pointer_cast<Point2D>(Point2DData.Get(DataPtNum))->m_x = std::dynamic_pointer_cast<Point2D>(Point2DData.Get(DataPtNum))->m_x + UniRealDist(Kaadugal::Randomizer::Get().GetRNG());
		    std::dynamic_pointer_cast<Point2D>(Point2DData.Get(DataPtNum))->m_y = std::dynamic_pointer_cast<Point2D>(Point2DData.Get(DataPtNum))->m_y + UniRealDist(Kaadugal::Randomizer::Get().GetRNG());
		    std::cout << "Test point (after perturb): " << std::dynamic_pointer_cast<Point2D>(Point2DData.Get(DataPtNum))->m_x << ", " << std::dynamic_pointer_cast<Point2D>(Point2DData.Get(DataPtNum))->m_y << std::endl;
		    std::shared_ptr<Kaadugal::AbstractDataPoint> TestPointPtr = std::dynamic_pointer_cast<Kaadugal::AbstractDataPoint>(Point2DData.Get(DataPtNum));
		    std::shared_ptr<HistogramStats> FinalStatsPtr = std::make_shared<HistogramStats>(HistogramStats(Point2DData.GetNumClasses()));
		    ForestBuilder.GetForest().Test(TestPointPtr, FinalStatsPtr);
		    std::cout << "Winner: " << FinalStatsPtr->FindWinnerLabelIndex() << std::endl;
		    std::cout << "Actual: " << std::dynamic_pointer_cast<Point2D>(Point2DData.Get(DataPtNum))->GetLabel() << std::endl;
		    if(FinalStatsPtr->FindWinnerLabelIndex() == std::dynamic_pointer_cast<Point2D>(Point2DData.Get(DataPtNum))->GetLabel())
			SuccessCtr++;
		}
		std::cout << "Classification Accuracy: " << float(SuccessCtr) / float(Total) * 100.0 << std::endl;
	    }

	}
	else
	    std::cout << "[ ERROR ]: Unable to train forest." << std::endl;
    }
    else
	std::cout << "[ WARN ]: Tree testing not implemented yet." << std::endl;


    return 0;
}
