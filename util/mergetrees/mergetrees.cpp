#include <iostream>
#include <fstream>

#include "Kaadugal.hpp"
#include "DecisionForestBuilder.hpp"

std::vector<std::string> g_TreeFNames;
std::string g_OutputForestFName;

// This program concatenates multiple trees into a single forest.
// This works because of the specific way the de-/serialization is done with
// the forests and trees
int main(int argc, char * argv[])
{
    if(argc < 3)
    {
	std::cout << "[ USAGE ]: " << argv[0] << " <TREE1_PATH> <TREE2_PATH> ... <OUTPUT_FOREST_FILE>" << std::endl;
	return -1;
    }

    for(int i = 1; i < argc-1; ++i)
	g_TreeFNames.push_back(std::string(argv[i]));

    g_OutputForestFName = std::string(argv[argc-1]);

    std::filebuf FileBuf;
    FileBuf.open(g_OutputForestFName, std::ios::out | std::ios::binary | std::ios::trunc);
    if(FileBuf.is_open())
    {
	std::ostream OutForest(&FileBuf);
	int nTrees = g_TreeFNames.size();
	OutForest.write((const char *)(&nTrees), sizeof(int));
	for(int i = 0; i < nTrees; ++i)
	{
	    std::filebuf LocFileBuf;
	    LocFileBuf.open(g_TreeFNames[i], std::ios::in | std::ios::binary);
	    if(LocFileBuf.is_open())
	    {
		std::istream Tree(&LocFileBuf);
		OutForest << Tree.rdbuf();
	    }
	    else
	    {
		std::cout << "[ WARN ]: Unable to open existing tree file. Exiting." << std::endl;
		return -2;
	    }
	}
	FileBuf.close();
	std::cout << "Done merging trees into forest: " << g_OutputForestFName << std::endl;
    }
    else
    {
	std::cout << "[ WARN ]: Unable to open forest file. Exiting." << std::endl;
	return -2;
    }

    return 0;
}
