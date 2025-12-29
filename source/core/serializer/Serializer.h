#pragma once
#include <cereal/archives/binary.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>

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
					std::cout << "Data saved to file: " << data->GetName() << std::endl;
					success = true;
				}
				catch (const std::exception& e) {
					std::cerr << "Failed to save data: " << e.what() << std::endl;
				}
				outputFile.close();
			}
			else {
				std::cerr << "Failed to open file for saving data" << std::endl;
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
					std::cout << "Data loaded from file: " << data->GetName() << std::endl;
				}
				catch (const std::exception& e) {
					std::cerr << "Failed to load data: " << e.what() << std::endl;
					// Continue with default data
				}
				inputFile.close();
			}
			else {
				std::cout << "No saved data found" << std::endl;
			}
			return data;
		}

	};
}