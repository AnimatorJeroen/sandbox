#pragma once
#include "ITestScene.h"
#include "app/sceneLayer/Scene.h"
#include "app/sceneLayer/SceneManager.h"
#include "core/renderer/Renderer_OpenGL.h"
#include <memory>

class TestScene1 : public ITestScene
{
private:
	std::shared_ptr<SceneManager> _sceneManager;
	Core::DrawCommandRecorder _recorder;
	Core::Renderer_OpenGL _renderer;
	Core::Renderer_OpenGL::RenderTargetSpecs _renderSpecs;

	// Helper function to render the grid floor
	inline void RenderGrid(Core::DrawCommandRecorder& recorder, float gridSize = 2000.0f, float gridStep = 100.0f)
	{
		Core::ColorRGBA gridColor = {0.5f, 0.5f, 0.5f, 0.5f}; // Gray color
		Core::ColorRGBA axisColorX = {1.0f, 0.2f, 0.2f, 0.5f }; // Red for X axis
		Core::ColorRGBA axisColorZ = {0.2f, 0.2f, 1.0f, 0.5f }; // Blue for Z axis
		float thickness = 1.0f;
		
		float halfSize = gridSize * 0.5f;
		
		// Draw grid lines along X direction (parallel to X axis)
		for (float z = -halfSize; z <= halfSize; z += gridStep)
		{
			Core::ColorRGBA color = (z == 0.0f) ? axisColorZ : gridColor;
			recorder.Line(
				Core::Vec3{-halfSize, 0.0f, z},
				Core::Vec3{halfSize, 0.0f, z},
				thickness,
				color
			);
		}
		
		// Draw grid lines along Z direction (parallel to Z axis)
		for (float x = -halfSize; x <= halfSize; x += gridStep)
		{
			Core::ColorRGBA color = (x == 0.0f) ? axisColorX : gridColor;
			recorder.Line(
				Core::Vec3{x, 0.0f, -halfSize},
				Core::Vec3{x, 0.0f, halfSize},
				thickness,
				color
			);
		}
	}

public:
	inline explicit TestScene1(std::shared_ptr<SceneManager> sceneManager) : _sceneManager(sceneManager)
	{
	}

	inline void Setup() override
	{
		auto scene = _sceneManager->CreateNewScene("Test Scene 1", true);

	}
	
	inline void Teardown() override
	{
	}
	
	inline void Update(float deltaTime) override
	{
		static float pingpong = 0.0f;
		static float incr = 0.5f;

		auto scene = _sceneManager->GetActiveScene();
		if (!scene) return;

		scene->UpdateCameraMatrices(_renderSpecs.width, _renderSpecs.height);
		scene->UpdateMatrices();
		
		// Update FBX animations based on deltaTime
		scene->UpdateFbxPlayer(deltaTime);

		if (pingpong >= 1.0f || pingpong <= 0.0f) incr = -incr;
		pingpong = std::min(1.0f, std::max(0.0f, pingpong + incr * deltaTime));

		//int i = 0;
		//for (auto entity : scene->GetRegistry().view<Transform>()) {
		//	auto& transform = scene->GetRegistry().get<Transform>(entity);
		//	transform.Position.x = pingpong * 10.0f * i;
		//	i++;
		//}

	}

	inline void Render() override
	{
		_recorder.Clear();
		auto scene = _sceneManager->GetActiveScene();
		if (scene)
		{
			_renderer.SetViewMatrix(scene->GetViewMatrix());
			_renderer.SetProjectionMatrix(scene->GetProjectionMatrix());
			
			// Render the grid floor
			RenderGrid(_recorder);
			scene->Draw(_recorder);
		}

		_renderer.BeginFrame(_renderSpecs);
		_renderer.Submit(_recorder.GetCommandBuffer());
		_renderer.EndFrame();
	}

	inline void SetRenderSpecs(const Core::IRenderer::RenderTargetSpecs& specs) override
	{
		_renderSpecs = specs;
	}

	inline const Core::IRenderer::RenderTargetSpecs& GetRenderSpecs() const override
	{
		return _renderSpecs;
	}
};