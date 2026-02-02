#pragma once
#include <fstream>
#include <iostream>
#include <filesystem>
#include <type_traits>
#include "core/Logger.h"

namespace Core
{
	// Type trait to detect if a type has SaveToFile method (C++17 compatible)
	template<typename T, typename = void>
	struct has_save_to_file : std::false_type {};
	
	template<typename T>
	struct has_save_to_file<T, std::void_t<decltype(std::declval<const T&>().SaveToFile(std::declval<const std::string&>()))>> : std::true_type {};

	// Type trait to detect if a type has LoadFromFile method (C++17 compatible)
	template<typename T, typename = void>
	struct has_load_from_file : std::false_type {};
	
	template<typename T>
	struct has_load_from_file<T, std::void_t<decltype(std::declval<T&>().LoadFromFile(std::declval<const std::string&>()))>> : std::true_type {};

	// Helper variable templates
	template<typename T>
	constexpr bool HasSaveToFile = has_save_to_file<T>::value;
	
	template<typename T>
	constexpr bool HasLoadFromFile = has_load_from_file<T>::value;

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