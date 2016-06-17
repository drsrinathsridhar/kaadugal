#ifndef _ABSTRACTFEATURERESPONSE_HPP_
#define _ABSTRACTFEATURERESPONSE_HPP_

#include <memory>
#include <ostream>
#include <istream>

#include "AbstractDataSet.hpp"

namespace Kaadugal
{
	class AbstractFeatureResponse
	{
	public:
		virtual VPFloat GetResponse(std::shared_ptr<AbstractDataPoint> DataPoint) = 0;

		virtual void Serialize(std::ostream& OutputStream) const = 0;
		virtual void Deserialize(std::istream& InputStream) = 0;

		virtual void Special(void) {};
	};
} // namespace Kaadugal

#endif // _ABSTRACTFEATURERESPONSE_HPP_
