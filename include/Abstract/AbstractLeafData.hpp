#ifndef _ABSTRACTLEAFDATA_HPP_
#define _ABSTRACTLEAFDATA_HPP_

#include <ostream>
#include <istream>

namespace Kaadugal
{
    class AbstractLeafData
    {
    public:
	virtual void Serialize(std::ostream& OutputStream) const {}; // TODO: Change to pure virtual
	virtual void Deserialize(std::istream& InputStream) {}; // TODO: Change to pure virtual
    };
} // namespace Kaadugal

#endif // _ABSTRACTLEAFDATA_HPP_
