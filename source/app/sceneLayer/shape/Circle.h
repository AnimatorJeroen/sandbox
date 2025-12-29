#pragma once
#include "IShape.h"
#include "glm/glm.hpp"
#include <cereal/archives/binary.hpp>
#include <cereal/types/polymorphic.hpp>

class Circle : public IShape
{
public:

	glm::vec2 center;
	float radius;
	bool filled;
	Core::ColorRGBA color;
	int num_segments;
	float thickness;
	
	Circle() = default;
	Circle(glm::vec2 center, float radius, bool filled, Core::ColorRGBA color, int num_segments, float thickness)
		: center(center), radius(radius), filled(filled), color(color), num_segments(num_segments), thickness(thickness) {}
	
	const void Draw(Core::DrawCommandRecorder& recorder) override;
	template<class Archive>
	void serialize(Archive& archive)
	{
		 archive(center.x, center.y, radius, filled, color.r, color.g, color.b, color.a, num_segments, thickness);
	}
};

CEREAL_REGISTER_TYPE(Circle)
CEREAL_REGISTER_POLYMORPHIC_RELATION(IShape, Circle)
