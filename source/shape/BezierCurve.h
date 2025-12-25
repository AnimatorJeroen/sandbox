#pragma once

#include "IShape.h"
#include "glm/glm.hpp"

class BezierCurve : IShape
{
public:
	struct Point
	{
		glm::vec2 pos;
		glm::vec2 hIn;
		glm::vec2 hOut;
	};

private:
	std::vector<Point> _points;

public:
	const void Draw(DrawCommandRecorder& recorder) override;

	void AddPoint(const glm::vec2& pos, const glm::vec2& hIn, const glm::vec2& hOut);

	inline Point& GetPoint(size_t index) { return _points[index]; }

	BezierCurve() = default;
};