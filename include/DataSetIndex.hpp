#pragma once

#include <memory>
#include <vector>
#include <iostream>

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
			if (uint64_t(Index.size()) > m_BaseDataSet->Size())
				std::cout << "[ WARN ]: Index size (" << Index.size() << ") exceeds dataset size (" << m_BaseDataSet->Size() << ")." << std::endl;
			m_Index = Index;
		};

		virtual int Size(void) { return int(m_Index.size()); };
		// Get index in original dataset
		int GetDataPointIndex(int i)
		{
			if (i >= Size())
				return -1;

			return m_Index[i];
		};

		std::shared_ptr<AbstractDataPoint> GetDataPoint(int i) { return m_BaseDataSet->Get(GetDataPointIndex(i)); };
		std::shared_ptr<AbstractDataSet> GetDataSet(void) { return m_BaseDataSet; };
		const std::vector<int>& GetIndex(void) { return m_Index; };
	};
} // namespace Kaadugal
