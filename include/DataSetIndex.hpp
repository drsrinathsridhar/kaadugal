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
	    if(Index.size() > m_BaseDataSet->Size())
		std::cout << "[ WARN ]: Index size (" << Index.size() << ") exceeds dataset size (" << m_BaseDataSet->Size() << ")." << std::endl;
	    m_Index = Index;
	};

	virtual int Size(void) { return m_Index.size(); };

	std::shared_ptr<AbstractDataPoint> GetDataPoint(int i)
	{
	    if(i >= Size())
		return nullptr;

	    return m_BaseDataSet->Get(m_Index[i]);
	};
    };

} // namespace Kaadugal

#endif // _DATASETINDEX_HPP_
