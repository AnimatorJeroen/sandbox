#pragma once
#include <vector>
#include "shape/IShape.h"
#include "shape/Circle.h"
#include "shape/BezierCurve.h"
#include <memory>

class Scene
{
	public:
		Scene() = default;
		std::vector<std::shared_ptr<IShape>> shapes;
		inline void Draw(DrawCommandRecorder& recorder) {
			for (const auto& shape : shapes) {
				shape->Draw(recorder);
			}
		}
};