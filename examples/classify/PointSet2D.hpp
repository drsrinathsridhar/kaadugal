#ifndef _POINTSET2D_HPP_
#define _POINTSET2D_HPP_

#include <fstream>
#include <sstream>
#include <iostream>
#include <ostream>
#include <string>
#include <set>

#include "Abstract/AbstractDataSet.hpp"

class Point2D : public Kaadugal::AbstractDataPoint
{
public:
    Point2D(void) {};
    Point2D(Kaadugal::VPFloat x, Kaadugal::VPFloat y, int ClassLabel = -1)
    {
	m_x = x;
	m_y = y;

	m_ClassLabel = ClassLabel;
    };

    Kaadugal::VPFloat m_x;
    Kaadugal::VPFloat m_y;

    int m_ClassLabel; // Always integer starting with 0. -1 means no class label

    const int& GetLabel(void) { return m_ClassLabel; };

    friend std::ostream& operator<<(std::ostream &s, const Point2D &p)
    {
	s << "[ " << p.m_x << ", " << p.m_y << " ]: " << p.m_ClassLabel << "\n";
	return s;
    };

    // virtual void Serialize(std::ostream& OutputStream) override
    // {
    // 	OutputStream.write((const char *)(&m_x), sizeof(Kaadugal::VPFloat));
    // 	OutputStream.write((const char *)(&m_y), sizeof(Kaadugal::VPFloat));
    // 	OutputStream.write((const char *)(&m_ClassLabel), sizeof(int));
    // };
    // virtual void Deserialize(std::istream& InputStream) override
    // {
    // 	InputStream.read((char *)(&m_x), sizeof(Kaadugal::VPFloat));
    // 	InputStream.read((char *)(&m_y), sizeof(Kaadugal::VPFloat));
    // 	InputStream.read((char *)(&m_ClassLabel), sizeof(int));
    // };
};

class PointSet2D : public Kaadugal::AbstractDataSet
{
private:
    int m_NumClassLabels;

public:
    PointSet2D(void) {};
    PointSet2D(const std::string& DataFileName)
    {
	// This is in human-readable format
	std::filebuf DataFile;
	DataFile.open(DataFileName, std::ios::in);
	if(DataFile.is_open())
	{
	    std::istream OutputFileStream(&DataFile);
	    Deserialize(OutputFileStream);
	    DataFile.close();
	}
	else
	    std::cout << "[ WARN ]: Unable to open file: " << DataFileName << std::endl;
    };

    const int& GetNumClasses(void) { return m_NumClassLabels; };

    virtual void Serialize(std::ostream& OutputStream) override
    {
	// This is in human-readable format
	// TODO:
    };
    virtual void Deserialize(std::istream& InputStream) override
    {
	// This is in human-readable format
	// PointSet2D data files have either 2 or 3 columns per element in the set
	// First Col: x; Second Col: y[, Third Col: Class Label]
	std::string Line;
	std::set<int> ClassSet;
	while(std::getline(InputStream, Line))
	{
	    // Skip empty lines or lines beginning with #
	    if(Line.size() < 1)
		continue;
	    if(Line.data()[0] == '#') // Comment lines
		continue;
	    
	    std::stringstream ss(Line);
	    std::vector<std::string> Cols;
	    std::string Tmp;
	    while(ss >> Tmp)
		Cols.push_back(Tmp);

	    Point2D Row;
	    if(Cols.size() == 2)
	    {
		Row = Point2D(std::atof(Cols[0].c_str())
			      , std::atof(Cols[1].c_str())
		    );
	    }
	    else if(Cols.size() == 3)
	    {	
		int ClassLabel = std::atoi(Cols[2].c_str());  // Input labels are ALWAYS indexed from 0
		ClassSet.insert(ClassLabel);
		Row = Point2D(std::atof(Cols[0].c_str())
			      , std::atof(Cols[1].c_str())
			      , ClassLabel
		    );
	    }
	    else
		throw std::runtime_error("Invalid data file. Exiting.");

	    // std::cout << Row;
	    m_DataPoints.push_back(std::make_shared<Point2D>(Row));
	}
	m_NumClassLabels = ClassSet.size();
	m_NumDataPoints = m_DataPoints.size();
	std::cout << "[ INFO ]: Finished reading input data (Total: " << m_NumDataPoints << "). Total number of classes: " << m_NumClassLabels << std::endl;
    };
};

#endif // _POINTSET2D_HPP_
