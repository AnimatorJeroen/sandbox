#pragma once

#include "IShape.h"
#include "glm/glm.hpp"
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/polymorphic.hpp>

class BezierCurve : public IShape
{
public:
	struct Point
	{
		glm::vec2 pos;
		glm::vec2 hIn;
		glm::vec2 hOut;

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(pos.x, pos.y, hIn.x, hIn.y, hOut.x, hOut.y);
		}
	};

private:
	std::vector<Point> _points;

public:
	const void Draw(Core::DrawCommandRecorder& recorder) override;

	void AddPoint(const glm::vec2& pos, const glm::vec2& hIn, const glm::vec2& hOut);

	inline Point& GetPoint(size_t index) { return _points[index]; }

	BezierCurve() = default;

	template<class Archive>
	void serialize(Archive& archive)
	{
		 archive(_points);
	}
};

CEREAL_REGISTER_TYPE(BezierCurve)
CEREAL_REGISTER_POLYMORPHIC_RELATION(IShape, BezierCurve)
