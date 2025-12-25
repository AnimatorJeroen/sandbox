#pragma once
#include "IShape.h"
#include "glm/glm.hpp"

class Circle : public IShape
{
public:

	glm::vec2 center;
	float radius;
	bool filled;
	ColorRGBA color;
	int num_segments;
	float thickness;
	
	Circle() = default;
	Circle(glm::vec2 center, float radius, bool filled, ColorRGBA color, int num_segments, float thickness)
		: center(center), radius(radius), filled(filled), color(color), num_segments(num_segments), thickness(thickness) {}
	
	const virtual void Draw(DrawCommandRecorder& recorder) override;
};