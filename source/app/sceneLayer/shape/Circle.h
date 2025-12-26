#pragma once
#include "IShape.h"
#include "glm/glm.hpp"

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
};