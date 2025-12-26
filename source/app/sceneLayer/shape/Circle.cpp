#include "Circle.h"

const void Circle::Draw(Core::DrawCommandRecorder& recorder)
{
	recorder.Circle(
		{ center.x, center.y },
		radius,
		filled,
		{ color.r, color.g, color.b, color.a },
		num_segments,
		thickness
	);
}