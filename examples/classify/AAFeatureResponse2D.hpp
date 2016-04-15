#ifndef _AAFEATRERESPONSE2D_HPP_
#define _AAFEATRERESPONSE2D_HPP_

#include "Abstract/AbstractFeatureResponse.hpp"
#include "PointSet2D.hpp"
#include "Randomizer.hpp"

// 2D axis-aligned feature response
class AAFeatureResponse2D : public Kaadugal::AbstractFeatureResponse
{
private:
	// NOTE: If new members are added, remember to add them to serialize/deserialize
	std::uniform_int_distribution<int> m_UniDist; // Both inclusive
	int m_SelectedFeature;

	int SelectFeature(void)
	{
		return m_UniDist(Kaadugal::Randomizer::Get().GetRNG());
	};

public:
	AAFeatureResponse2D(void)
		: m_UniDist(0, 1) /* Both inclusive */
	{
		m_SelectedFeature = SelectFeature();
	};

	int GetSelectedFeature(void) { return m_SelectedFeature; };

	virtual Kaadugal::VPFloat GetResponse(std::shared_ptr<Kaadugal::AbstractDataPoint> DataPoint) override
	{
		std::shared_ptr<Point2D> PointIn2D = std::dynamic_pointer_cast<Point2D>(DataPoint);

		return (m_SelectedFeature == 0 ? PointIn2D->m_x : PointIn2D->m_y);
	};

	virtual void Serialize(std::ostream& OutputStream) const override
	{
		OutputStream.write((const char *)(&m_SelectedFeature), sizeof(int));
	};
	virtual void Deserialize(std::istream& InputStream) override
	{
		InputStream.read((char *)(&m_SelectedFeature), sizeof(int));
	};
};

#endif // _AAFEATRERESPONSE2D_HPP_
