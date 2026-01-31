#pragma once
#include <fstream>
#include <iostream>
#include <filesystem>
#include <type_traits>
#include "core/Logger.h"

namespace Core
{
	// Type trait to detect if a type has SaveToFile method
	template<typename T>
	concept HasSaveToFile = requires(const T& t, const std::string& path) {
		{ t.SaveToFile(path) } -> std::convertible_to<bool>;
	};

	// Type trait to detect if a type has LoadFromFile method
	template<typename T>
	concept HasLoadFromFile = requires(T& t, const std::string& path) {
		{ t.LoadFromFile(path) } -> std::convertible_to<bool>;
	};

	class Serializer
	{
	public:
		// Specialized for types with SaveToFile method (like Scene)
		template<typename T>
		inline static bool Serialize(const std::shared_ptr<T>& data, const std::string& filepath) {
			if constexpr (HasSaveToFile<T>) {
				return data->SaveToFile(filepath);
			} else {
				LOG_ERROR() << "Serialization not implemented for this type";
				return false;
			}
		}

		// Specialized for types with LoadFromFile method (like Scene)
		template<typename T>
		inline static std::shared_ptr<T> Deserialize(const std::string& filepath) {
			if constexpr (HasLoadFromFile<T>) {
				std::shared_ptr<T> data = std::make_shared<T>();
				if (data->LoadFromFile(filepath)) {
					LOG_DEBUG() << "Data loaded from file: " << filepath;
					return data;
				} else {
					LOG_ERROR() << "Failed to load data from: " << filepath;
					return nullptr;
				}
			} else {
				LOG_ERROR() << "Deserialization not implemented for this type";
				return nullptr;
			}
		}
	};
}