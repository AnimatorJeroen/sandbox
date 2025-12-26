#pragma once
#include <vector>
#include <memory>
#include "shape/IShape.h"
#include "shape/Circle.h"
#include "shape/BezierCurve.h"

class Scene
{
	public:
		Scene() = default;
		std::vector<std::shared_ptr<IShape>> shapes;
		inline void Draw(Core::DrawCommandRecorder& recorder) {
			for (const auto& shape : shapes) {
				shape->Draw(recorder);
			}
		}
};