#include <iostream>

#include "Kaadugal.hpp"
#include "DecisionForestBuilder.hpp"

int main(int argc, char * argv[])
{
    // Load parameters from file
    Kaadugal::ForestBuilderParameters ForestParams("test.param");
    Kaadugal::AbstractDataSet AbsTestData; // TODO: Read this from disk

    // Build forest from training data
    Kaadugal::DecisionForestBuilder<Kaadugal::AbstractFeatureResponse
				    , Kaadugal::AbstractStatistics
				    , Kaadugal::AbstractLeafData
				    , Kaadugal::AbstractDataSet
				    , Kaadugal::AbstractDataSetIndex> ForestBuilder(ForestParams);
    if(ForestBuilder.Build(AbsTestData) == true)
    {
	std::cout << "Random Forest successfully trained." << std::endl;
	int t;
	std::cin >> t;
    }
    else
	std::cout << "[ ERROR ]: Unable to train forest." << std::endl;

    
    return 0;
}
