#pragma once

#include "IShape.h"
#include "glm/glm.hpp"

class Beziercurve : IShape
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