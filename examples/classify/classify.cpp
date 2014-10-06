#include <iostream>

#include "Kaadugal.hpp"
#include "DecisionTreeBuilder.hpp"

int main(int argc, char * argv[])
{
    // Load parameters from file
    Kaadugal::TreeBuilderParameters ForestParams("test.param");
    
    Kaadugal::DecisionTree<Kaadugal::AbstractFeatureResponse, Kaadugal::AbstractStatistics, Kaadugal::AbstractLeafData> TestTree(10);

    return 0;
}
