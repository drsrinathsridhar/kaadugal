#pragma once

#include <ostream>
#include <istream>
#include <memory>

#include "DataSetIndex.hpp"

namespace Kaadugal
{
	// AbstractLeafData is a class for storing arbitrary information at the leaf nodes
	// Typically, this arbitrary information is a function of the datapoints that reach
	// a leaf (including the identity function a.k.a. data points are the arbitrary data)
	class AbstractLeafData
	{
	protected:
		bool m_isConstructed;

	public:
		// TODO: Not using pure virtual functions to avoid breaking previous versions that use Kaadugal
		virtual void Serialize(std::ostream& OutputStream) const {};
		virtual void Deserialize(std::istream& InputStream) {};

		bool isConstructed(void) { return m_isConstructed; };

		// Member that constructs leaf data based on whatever logic. Takes dataset index as input
		virtual void Construct(std::shared_ptr<DataSetIndex> LeafDataSetIdx) {};
	};
} // namespace Kaadugal
