#pragma once
#include <cereal/archives/binary.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>
#include "core/Logger.h"

namespace Core
{

	class Serializer
	{
	public:
		template<class T>
		inline static bool Serialize(const std::shared_ptr<T>& data, const std::string& filepath) {

			std::filesystem::path filePath(filepath);
			std::filesystem::create_directories(filePath.parent_path());
			bool success = false;

			std::ofstream outputFile(filepath, std::ios::binary);
			if (outputFile.good()) {
				try {
					cereal::BinaryOutputArchive outputArchive(outputFile);
					outputArchive(*data);
					LOG_DEBUG() << "Data saved to file: " << data->GetName();
					success = true;
				}
				catch (const std::exception& e) {
					LOG_ERROR() << "Failed to save data : " << e.what();
				}
				outputFile.close();
			}
			else {
				LOG_ERROR() << "Failed to open file for saving data";
			}
			return success;
		}

		template<class T>
		inline static std::shared_ptr<T> Deserialize(const std::string& filepath) {

			// Load data from file if it exists
			std::shared_ptr<T> data = std::make_shared<T>();
			std::ifstream inputFile(filepath, std::ios::binary);
			if (inputFile.good()) {
				try {
					cereal::BinaryInputArchive inputArchive(inputFile);
					inputArchive(*data);
                    LOG_DEBUG() << "Data loaded from file: " << data->GetName();
				}
				catch (const std::exception& e) {
					LOG_ERROR() << "Failed to load data: " << e.what();
					data = nullptr;
				}
				inputFile.close();
			}
			else {
				LOG_DEBUG() << "No saved data found";
				data = nullptr;
			}
			return data;
		}

	};
}