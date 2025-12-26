#include "BezierCurve.h"

const void BezierCurve::Draw(Core::DrawCommandRecorder& recorder)
{
	if (_points.size() < 2)
		return;

	Point const* pt = nullptr;
	Point const* nextPt = nullptr;
	for (size_t i = 0; i < _points.size() - 1; ++i)
	{
		pt = &_points[i];
		nextPt = &_points[i + 1];
		recorder.CubicBezier(
			{ pt->pos.x, pt->pos.y },
			{ pt->pos.x + pt->hOut.x,  pt->pos.y + pt->hOut.y },
			{ nextPt->pos.x + nextPt->hIn.x,  nextPt->pos.y + nextPt->hIn.y },
			{ nextPt->pos.x, nextPt->pos.y },
			2.0f,
			{ 1.0f, 0.0f, 0.0f, 1.0f }
		);
	}
}

void BezierCurve::AddPoint(const glm::vec2& pos, const glm::vec2& hIn, const glm::vec2& hOut) {
	_points.push_back({ pos, hIn, hOut });
}

