#pragma once
#include <vector>
#include <memory>
#include <string>
#include "shape/IShape.h"
#include "shape/Circle.h"
#include "shape/BezierCurve.h"
#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>

class Scene
{
	public:
		Scene() = default;
		
		inline void Draw(Core::DrawCommandRecorder& recorder) {
			for (const auto& shape : _shapes) {
				shape->Draw(recorder);
			}
		}
		void SetName(const std::string& name) { _name = name; }
		std::string& GetName() { return _name; }

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(_name, _shapes);
		}

		std::vector<std::shared_ptr<IShape>>& GetShapes() { return _shapes; }

	private:
		std::string _name;
		std::vector<std::shared_ptr<IShape>> _shapes;
		int a = 0;
};