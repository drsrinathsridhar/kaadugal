#include <iostream>

#include "Kaadugal.hpp"
#include "DecisionForestBuilder.hpp"

int main(int argc, char * argv[])
{
    // Load parameters from file
    Kaadugal::ForestBuilderParameters ForestParams("test.param");
    Kaadugal::AbstractDataSet AbsTestData; // TODO: Read this from disk
    std::shared_ptr<Kaadugal::AbstractDataSet> TestDataPtr = std::make_shared<Kaadugal::AbstractDataSet>(AbsTestData);

    // Build forest from training data
    Kaadugal::DecisionForestBuilder<Kaadugal::AbstractFeatureResponse
				    , Kaadugal::AbstractStatistics
				    , Kaadugal::AbstractLeafData> ForestBuilder(ForestParams);
    if(ForestBuilder.Build(TestDataPtr) == true)
    {
	std::cout << "Random Forest successfully trained." << std::endl;
	int t;
	std::cin >> t;
    }
    else
	std::cout << "[ ERROR ]: Unable to train forest." << std::endl;

    
    return 0;
}
