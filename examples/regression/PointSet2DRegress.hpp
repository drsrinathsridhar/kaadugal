#pragma once 

#include <fstream>
#include <sstream>
#include <iostream>
#include <ostream>
#include <string>
#include <set>

#include "Abstract/AbstractDataSet.hpp"

class Point2DRegress : public Kaadugal::AbstractDataPoint
{
public:
	Point2DRegress(void)
	{
		m_x = 0.0;
		m_y = 0.0;
		m_Value = NAN;
	};
	Point2DRegress(Kaadugal::VPFloat x, Kaadugal::VPFloat y, Kaadugal::VPFloat Val = NAN)
	{
		m_x = x;
		m_y = y;

		m_Value = Val;
	};

	Kaadugal::VPFloat m_x;
	Kaadugal::VPFloat m_y;

	Kaadugal::VPFloat m_Value; // Any float value, nan is invalid

	int GetValue(void) { return m_Value; };

	friend std::ostream& operator<<(std::ostream &s, const Point2DRegress &p)
	{
		s << "[ " << p.m_x << ", " << p.m_y << " ]: " << p.m_Value << "\n";
		return s;
	};

	Kaadugal::VPFloat& operator[](int Idx)
	{
		if (Idx >= 2 || Idx < 0)
			throw std::runtime_error("Exceeded dimensions.");

		if (Idx == 0)
			return m_x;

		return m_y;
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

class PointSet2DRegress : public Kaadugal::AbstractDataSet
{
public:
	PointSet2DRegress(void) {};
	PointSet2DRegress(const std::string& DataFileName)
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

	virtual void Serialize(std::ostream& OutputStream) override
	{
		// This is in human-readable format
		// TODO:
	};
	virtual void Deserialize(std::istream& InputStream) override
	{
		// This is in human-readable format
		// PointSet2DRegress data files have 3 columns per element in the set
		// First Col: x; Second Col: y, Third Col: Regression value
		std::string Line;
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

			Point2DRegress Row;
			if (Cols.size() != 3)
				throw std::runtime_error("Unable to read data files. Must be 3 columns");

			Kaadugal::VPFloat Val = std::atof(Cols[2].c_str());
			Row = Point2DRegress(std::atof(Cols[0].c_str())
				, std::atof(Cols[1].c_str())
				, Val
				);

			// std::cout << Row;
			m_DataPoints.push_back(std::make_shared<Point2DRegress>(Row));
		}
		m_NumDataPoints = m_DataPoints.size();
		std::cout << "[ INFO ]: Finished reading input data (Total: " << m_NumDataPoints << ")" << std::endl;
	};
};
