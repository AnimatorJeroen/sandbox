#pragma once

#include "Shape.h"
#include "glm/glm.hpp"

class Beziercurve : Shape
{
public:
	struct Point
	{
		glm::vec2 pos;
		glm::vec2 hIn;
		glm::vec2 hOut;
	};

private:
	std::vector<Point> points;

public:
	const void Draw() override;
};