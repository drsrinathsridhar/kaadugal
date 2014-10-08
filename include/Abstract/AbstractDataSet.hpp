#ifndef _ABSTRACTDATASET_HPP_
#define _ABSTRACTDATASET_HPP_

#include <memory>

namespace Kaadugal
{
    class AbstractDataSet;
    class AbstractDataSetIndex
    {
    protected:
	std::shared_ptr<AbstractDataSet> m_BaseDataSet;
	// Contains indices of data that an instance of this class has access to
        // By default, it has access to nothing
	std::vector<int> m_Index;

    public:
	AbstractDataSetIndex(std::shared_ptr<AbstractDataSet> DataSet, const std::vector<int>& Index)
	{
	    m_BaseDataSet = DataSet;
	    m_Index = Index;
	};
    };

    class AbstractDataSet
    {
    public:
	// int Size(void) = 0; // TODO: This must be the final method
	int Size(void) { return 10; };
    };
} // namespace Kaadugal

#endif // _ABSTRACTDATASET_HPP_
