#include "core/Application.h"
#include "app/sceneLayer/SceneApplicationLayer.h"
#include "app/UILayer/UIApplicationLayer.h"
#include <cereal/archives/binary.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>

int main() {

	Core::ApplicationSpecs specs {
		800,
		600,
		"Application Skeleton"
	};
	Core::Application app(specs);
	
	// Load scene from file if it exists
	std::shared_ptr<Scene> scene = std::make_shared<Scene>();
	const std::string sceneFilePath = "saved files/scene.dat";
	
	std::ifstream inputFile(sceneFilePath, std::ios::binary);
	if (inputFile.good()) {
		try {
			cereal::BinaryInputArchive inputArchive(inputFile);
			inputArchive(*scene);
			std::cout << "Scene loaded from file: " << scene->GetName() << std::endl;
		}
		catch (const std::exception& e) {
			std::cerr << "Failed to load scene: " << e.what() << std::endl;
			// Continue with default scene
		}
		inputFile.close();
	}
	else {
		std::cout << "No saved scene found, starting with new scene" << std::endl;
	}
	
	app.GetContext().Register<Scene>(scene);
	app.PushLayer<UIApplicationLayer>();
	app.PushLayer<SceneApplicationLayer>();
    app.Run();

	// Save scene to file after application closes
	std::filesystem::path filePath(sceneFilePath);
	std::filesystem::create_directories(filePath.parent_path());

	std::ofstream outputFile(sceneFilePath, std::ios::binary);
	if (outputFile.good()) {
		try {
			cereal::BinaryOutputArchive outputArchive(outputFile);
			outputArchive(*scene);
			std::cout << "Scene saved to file: " << scene->GetName() << std::endl;
		}
		catch (const std::exception& e) {
			std::cerr << "Failed to save scene: " << e.what() << std::endl;
		}
		outputFile.close();
	}
	else {
		std::cerr << "Failed to open file for saving scene" << std::endl;
	}

    return 0;
}