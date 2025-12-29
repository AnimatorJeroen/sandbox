#pragma once
#include "ITestScene.h"
#include "app/sceneLayer/Scene.h"
#include "app/sceneLayer/SceneManager.h"
#include "core/renderer/Renderer_ImGui.h"
#include <memory>
#include <app/sceneLayer/shape/Circle.h>
#include <app/sceneLayer/shape/BezierCurve.h>

class TestScene1 : public ITestScene
{
private:
	std::shared_ptr<SceneManager> _sceneManager;
	Core::DrawCommandRecorder _recorder;
	Core::Renderer_ImGui _renderer;
	Core::Renderer_ImGui::RenderTargetSpecs _renderSpecs;

public:
	inline explicit TestScene1(std::shared_ptr<SceneManager> sceneManager) : _sceneManager(sceneManager)
	{
	}

	inline void Setup() override
	{
		auto scene = _sceneManager->CreateNewScene("Test Scene 1", true);

		std::shared_ptr<Circle> circle = std::make_shared<Circle>(
			glm::vec2{ 100.0f, 100.0f },  // center
			50.0f,                        // radius
			true,                         // filled
			Core::ColorRGBA{ 0.0f, 0.0f, 1.0f, 1.0f },  // color
			32,                           // num_segments
			1.0f                          // thickness
		);

		std::shared_ptr<BezierCurve> bezierCurve = std::make_shared<BezierCurve>();

		bezierCurve->AddPoint({ 300.0f, 300.0f }, { -50.0f, 0.0f }, { 50.0f, 0.0f });
		bezierCurve->AddPoint({ 400.0f, 400.0f }, { -50.0f, 0.0f }, { 50.0f, 0.0f });
		bezierCurve->AddPoint({ 500.0f, 300.0f }, { -50.0f, 0.0f }, { 0.0f, 0.0f });

		scene->GetShapes().push_back(circle);
		scene->GetShapes().push_back(bezierCurve);
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

		auto it = std::find_if(scene->GetShapes().begin(), scene->GetShapes().end(),
			[](const std::shared_ptr<IShape>& shape) {
				return dynamic_cast<BezierCurve*>(shape.get()) != nullptr;
			});
		if (it != scene->GetShapes().end()) {
			auto bezierCurve = static_cast<BezierCurve*>(it->get());
			bezierCurve->GetPoint(1).pos.x = 400.0f + 100.0f * pingpong;
		}
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