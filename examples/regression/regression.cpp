#include <iostream>
#include <fstream>

#include "Kaadugal.hpp"
#include "DecisionForestBuilder.hpp"
#include "PointSet2DRegress.hpp"
#include "AAFeatureResponse2D.hpp"
#include "GaussianStats.hpp"

bool g_isTrainMode;
std::string g_DataFileName;
PointSet2DRegress g_Point2DData;

// Training members
// train ..\examples\config\regress_2d.train testreg.forest ..\examples\data\TrainData2D_Regress_2000.dat
std::string g_ParamFileName;
std::string g_OutputForestFName;

// Testing members
//  test testreg.forest ..\examples\data\TestData2D_Regress_200.dat
std::string g_InputForestFName;

void PrintUsage(char * argv[])
{
	std::cout << "[ USAGE ]: " << argv[0] << " (train <CONFIG_FILE_PATH> <OUTPUT_FOREST_PATH> | test <INPUT_FOREST_PATH>)  <DATA_FILE>" << std::endl;
}

bool ParseArguments(int argc, char * argv[])
{
	if (argc < 4 || argc > 5)
	{
		PrintUsage(argv);
		return false;
	}
	if (std::string(argv[1]) != "train" && std::string(argv[1]) != "test")
	{
		PrintUsage(argv);
		return false;
	}

	if (std::string(argv[1]) == "train" && argc == 5)
	{
		g_ParamFileName = argv[2];
		g_OutputForestFName = argv[3];
		g_DataFileName = argv[4];
		g_isTrainMode = true;
		return true;
	}
	if (std::string(argv[1]) == "test" && argc == 4)
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
	std::shared_ptr<Kaadugal::AbstractDataSet> Point2DDataPtr = std::make_shared<PointSet2DRegress>(g_Point2DData);

	// Build forest from training data
	if (ForestParams.m_NumTrees == 1) // We need to build only one tree
	{
		Kaadugal::DecisionTreeBuilder<AAFeatureResponse2D, GaussianStats> TreeBuilder(ForestParams);
		if (TreeBuilder.Build(Point2DDataPtr) == true)
		{
			std::cout << "One tree of the random forest successfully trained." << std::endl;

			std::cout << "Writing tree to file..." << std::endl;
			std::filebuf FileBuf;
			FileBuf.open(g_OutputForestFName + ".tree", std::ios::out | std::ios::trunc | std::ios::binary);
			if (FileBuf.is_open())
			{
				std::ostream OutTree(&FileBuf);
				TreeBuilder.GetTree()->Serialize(OutTree);
				FileBuf.close();
				std::cout << "Done. You can merge the trees using the mergetrees program." << std::endl;
			}
			else
				std::cout << "[ WARN ]: Unable to open file to save." << std::endl;
		}
		else
			std::cout << "[ ERROR ]: Unable to train tree." << std::endl;
	}
	else
	{
		Kaadugal::DecisionForestBuilder<AAFeatureResponse2D, GaussianStats> ForestBuilder(ForestParams);
		if (ForestBuilder.Build(Point2DDataPtr) == true)
		{
			std::cout << "Random Forest successfully trained." << std::endl;

			std::cout << "Writing forest to file..." << std::endl;
			std::filebuf FileBuf;
			FileBuf.open(g_OutputForestFName, std::ios::out | std::ios::trunc | std::ios::binary);
			if (FileBuf.is_open())
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
	}

	return 0;
}

int Test(void)
{
	Kaadugal::DecisionForest<AAFeatureResponse2D, GaussianStats, Kaadugal::AbstractLeafData> LoadedForest;
	std::cout << "Loading forest from file..." << std::endl;
	std::filebuf FileBuf;
	FileBuf.open(g_InputForestFName, std::ios::in | std::ios::binary);
	if (FileBuf.is_open())
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
	int Ctr = 0;
	int DataSize = g_Point2DData.Size();
	float Dist = 0.0;
	for (int i = 0; i < DataSize; ++i)
	{
		std::shared_ptr<Kaadugal::AbstractDataPoint> TestPointPtr = std::dynamic_pointer_cast<Kaadugal::AbstractDataPoint>(g_Point2DData.Get(i));
		std::shared_ptr<GaussianStats> FinalStatsPtr = std::shared_ptr<GaussianStats>(new GaussianStats);
		LoadedForest.Test(TestPointPtr, FinalStatsPtr);
		auto BestVal = FinalStatsPtr->GetMeanParams().at<float>(0, 0);
		auto ActualVal = std::dynamic_pointer_cast<Point2DRegress>(g_Point2DData.Get(i))->m_Value;
		//std::cout << "Best: " << BestVal << std::endl;
		//std::cout << "Actual: " << ActualVal << std::endl;
		Dist += (ActualVal - BestVal) * (ActualVal - BestVal);
		Ctr++;
	}
	std::cout << "Average Error: " << sqrt(Dist / float(Ctr)) << std::endl;

	return 0;
}

int main(int argc, char * argv[])
{
	if (ParseArguments(argc, argv) == false)
		return -1;

	g_Point2DData = PointSet2DRegress(g_DataFileName);
	if (g_isTrainMode)
		return Train();
	else
		return Test();

	return 0;
}
