#pragma once
#include <vector>
#include <memory>
#include <string>

#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include "shape/IShape.h"

class Scene
{
	public:
		Scene() = default;
		
		void Draw(Core::DrawCommandRecorder& recorder);
		void SetName(const std::string& name);
		std::string& GetName();

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(_name, _shapes);
		}

		inline std::vector<std::shared_ptr<IShape>>& GetShapes() { return _shapes; }

	private:
		std::string _name;
		std::vector<std::shared_ptr<IShape>> _shapes;
		int a = 0;
};