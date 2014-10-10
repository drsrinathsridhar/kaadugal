#ifndef _ABSTRACTDATASET_HPP_
#define _ABSTRACTDATASET_HPP_

#include <memory>

namespace Kaadugal
{
    class AbstractDataPoint
    {
    public:
	virtual ~AbstractDataPoint(void) {};
    };

    class AbstractDataSet
    {
    protected:
	std::vector<std::shared_ptr<AbstractDataPoint>> m_DataPoints;

    public:
	virtual int Size(void) { return m_DataPoints.size(); };
	virtual std::shared_ptr<AbstractDataPoint> Get(int i)
	{
	    if(i < 0)
		return nullptr;

	    return m_DataPoints[i];
	};
    };
} // namespace Kaadugal

#endif // _ABSTRACTDATASET_HPP_
