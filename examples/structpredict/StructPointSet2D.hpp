#ifndef _STRUCTPOINTSET2D_HPP_
#define _STRUCTPOINTSET2D_HPP_

#include <fstream>
#include <sstream>
#include <iostream>
#include <ostream>
#include <string>
#include <set>

#include "Abstract/AbstractDataSet.hpp"

// Supports structured labels of dimension 2
class StructPoint2D : public Kaadugal::AbstractDataPoint
{
public:
	StructPoint2D(void)
	{

	};
	StructPoint2D(const Kaadugal::VPFloat& x, const Kaadugal::VPFloat& y, const std::pair<int, int>& StructLabel)
		: m_x(x)
		, m_y(y)
		, m_StructLabel(StructLabel)
	{

	};

	Kaadugal::VPFloat m_x;
	Kaadugal::VPFloat m_y;

	std::pair<int, int> m_StructLabel; // Each dimension in the structured label is always integer starting with 0. -1 means no class label

	friend std::ostream& operator<<(std::ostream &s, const StructPoint2D &p)
	{
		s << "[ " << p.m_x << ", " << p.m_y << " ]: " << p.m_StructLabel.first << ", " << p.m_StructLabel.second << "\n";

		return s;
	};

	virtual void Serialize(std::ostream& OutputStream) const override
	{
		//OutputStream.write((const char *)(&m_x), sizeof(Kaadugal::VPFloat));
		//OutputStream.write((const char *)(&m_y), sizeof(Kaadugal::VPFloat));
		//OutputStream.write((const char *)(&m_Value), sizeof(int));

		std::cout << "[ WARN ]: Serialize() - Not yet implemented." << std::endl;
	};
	virtual void Deserialize(std::istream& InputStream) override
	{
		//InputStream.read((char *)(&m_x), sizeof(Kaadugal::VPFloat));
		//InputStream.read((char *)(&m_y), sizeof(Kaadugal::VPFloat));
		//InputStream.read((char *)(&m_Value), sizeof(int));

		std::cout << "[ WARN ]: Deserialize() - Not yet implemented." << std::endl;
	};
};

class StructPointSet2D : public Kaadugal::AbstractDataSet
{
private:
	int m_ClassesPerDim; // Number of class labels in each structured label dimension

public:
	StructPointSet2D(void) {};
	StructPointSet2D(const std::string& DataFileName)
	{
		// This is in human-readable format
		std::filebuf DataFile;
		DataFile.open(DataFileName, std::ios::in);
		if (DataFile.is_open())
		{
			std::istream OutputFileStream(&DataFile);
			Deserialize(OutputFileStream);
			DataFile.close();
		}
		else
			std::cout << "[ WARN ]: Unable to open file: " << DataFileName << std::endl;
	};

	const int& GetNumClasses(void) { return m_ClassesPerDim; };

	virtual void Serialize(std::ostream& OutputStream) override
	{
		// This is in human-readable format
		// TODO:
	};
	virtual void Deserialize(std::istream& InputStream) override
	{
		// This is in human-readable format
		// StructPointSet2D data files have 2 data columns for (x, y) positions
		// followed by two columns for the structured label
		// First Col: x Second Col: y StructuredClassLabel(-1 if dummy)
		std::string Line;
		std::set<int> ClassSet;
		while (std::getline(InputStream, Line))
		{
			// Skip empty lines or lines beginning with #
			if (Line.size() < 1)
				continue;
			if (Line.data()[0] == '#') // Comment lines
				continue;

			std::stringstream ss(Line);
			std::vector<std::string> Cols;
			std::string Tmp;
			while (ss >> Tmp)
				Cols.push_back(Tmp);

			StructPoint2D Row;
			if (Cols.size() == 2)
			{
				std::pair<int, int> StructLabel;
				StructLabel = std::make_pair(-1, -1);
				Row = StructPoint2D(std::atof(Cols[0].c_str())
					, std::atof(Cols[1].c_str())
					, StructLabel
					);
			}
			else if (Cols.size() == 4)
			{
				std::pair<int, int> StructLabel;
				StructLabel = std::make_pair(std::atoi(Cols[2].c_str()), std::atoi(Cols[3].c_str()));
				ClassSet.insert(std::atoi(Cols[2].c_str()));
				ClassSet.insert(std::atoi(Cols[3].c_str()));

				Row = StructPoint2D(std::atof(Cols[0].c_str())
					, std::atof(Cols[1].c_str())
					, StructLabel
					);
			}
			else
				throw std::runtime_error("Invalid data file. Exiting.");

			// std::cout << Row;
			m_DataPoints.push_back(std::make_shared<StructPoint2D>(Row));
		}
		m_ClassesPerDim = ClassSet.size();
		std::cout << "[ INFO ]: Finished reading " << m_DataPoints.size() << " input data. Total number of classes per structured dimension is " << m_ClassesPerDim << std::endl;
	};
};

#endif // _STRUCTPOINTSET2D_HPP_
