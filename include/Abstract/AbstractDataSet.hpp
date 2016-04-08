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
		virtual void Serialize(std::ostream& OutputStream) = 0;
		virtual void Deserialize(std::istream& InputStream) = 0;
	};

	class AbstractDataSet
	{
	protected:
		std::vector<std::shared_ptr<AbstractDataPoint>> m_DataPoints;
		uint64_t m_NumDataPoints; // To manage large datasets we use uint64_t

	public:
		virtual void Serialize(std::ostream& OutputStream) = 0;
		virtual void Deserialize(std::istream& InputStream) = 0;
		virtual uint64_t Size(void) { return m_NumDataPoints; };
		virtual std::shared_ptr<AbstractDataPoint> Get(uint64_t i)
		{
			if (i >= m_NumDataPoints)
				return nullptr;

			return m_DataPoints[i];
		};
		virtual void Special(int NodeIndex = 0, const std::vector<int>& Index = std::vector<int>())
		{
			std::cout << "[ WARN ]: AbstractDataPoint::doSpecial() - Not implemented." << std::endl; 
		};
	};
} // namespace Kaadugal

#endif // _ABSTRACTDATASET_HPP_
