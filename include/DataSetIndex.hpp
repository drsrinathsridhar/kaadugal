#ifndef _DATASETINDEX_HPP_
#define _DATASETINDEX_HPP_

#include <memory>

#include "Abstract/AbstractDataSet.hpp"

namespace Kaadugal
{
    class DataSetIndex
    {
    protected:
	std::shared_ptr<AbstractDataSet> m_BaseDataSet;
	// Contains indices of data that an instance of this class has access to
	// By default, it has access to nothing
	std::vector<int> m_Index;

    public:
	DataSetIndex(std::shared_ptr<AbstractDataSet> DataSet, const std::vector<int>& Index)
	{
	    m_BaseDataSet = DataSet;
	    m_Index = Index;
	};
    };

} // namespace Kaadugal

#endif // _DATASETINDEX_HPP_
