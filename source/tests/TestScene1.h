#pragma once
#include "ITestScene.h"
#include "app/sceneLayer/Scene.h"
#include "app/sceneLayer/SceneManager.h"
#include "core/renderer/Renderer_ImGui.h"
#include "core/renderer/Renderer_OpenGL.h"
#include <memory>

class TestScene1 : public ITestScene
{
private:
	std::shared_ptr<SceneManager> _sceneManager;
	Core::DrawCommandRecorder _recorder;
	Core::Renderer_OpenGL _renderer;
	Core::Renderer_OpenGL::RenderTargetSpecs _renderSpecs;

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

		if (pingpong >= 1.0f || pingpong <= 0.0f) incr = -incr;
		pingpong = std::min(1.0f, std::max(0.0f, pingpong + incr * deltaTime));

	}

	inline void Render() override
	{
		auto scene = _sceneManager->GetActiveScene();
		if (!scene) return;

		_recorder.Clear();

		scene->Draw(_recorder);
		
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