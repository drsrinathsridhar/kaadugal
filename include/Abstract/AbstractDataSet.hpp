#ifndef _ABSTRACTDATASET_HPP_
#define _ABSTRACTDATASET_HPP_

#include <memory>
#include <ostream>
#include <istream>

namespace Kaadugal
{
    class AbstractDataPoint
    {
    public:
	virtual ~AbstractDataPoint(void) {};
	// virtual void Serialize(std::ostream& OutputStream) = 0;
	// virtual void Deserialize(std::istream& InputStream) = 0;
    };

    class AbstractDataSet
    {
    protected:
	std::vector<std::shared_ptr<AbstractDataPoint>> m_DataPoints;

    public:
	virtual void Serialize(std::ostream& OutputStream) = 0;
	virtual void Deserialize(std::istream& InputStream) = 0;
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
