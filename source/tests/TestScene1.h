#pragma once
#include "ITestScene.h"
#include "app/sceneLayer/Scene.h"
#include "core/renderer/Renderer_ImGui.h"

class TestScene1 : public ITestScene
{
private:
	Scene& _scene;
	Core::DrawCommandRecorder _recorder;
	Core::Renderer_ImGui _renderer;
	Core::Renderer_ImGui::RenderTargetSpecs _renderSpecs;

	float pingpong = 0.0f;
	float incr = 0.5f;


	std::shared_ptr<Circle> circle = std::make_shared<Circle>(
		glm::vec2{100.0f, 100.0f},  // center
		50.0f,                        // radius
		true,                         // filled
		Core::ColorRGBA{ 0.0f, 0.0f, 1.0f, 1.0f },  // color
		32,                           // num_segments
		1.0f                          // thickness
	);

	std::shared_ptr<BezierCurve> bezierCurve = std::make_shared<BezierCurve>();

public:

	inline explicit TestScene1(Scene& context) : _scene(context)
	{

	}

	inline void Setup() override
	{
		bezierCurve->AddPoint({ 300.0f, 300.0f }, { -50.0f, 0.0f }, { 50.0f, 0.0f });
		bezierCurve->AddPoint({ 400.0f, 400.0f }, { -50.0f, 0.0f }, { 50.0f, 0.0f });
		bezierCurve->AddPoint({ 500.0f, 300.0f }, { -50.0f, 0.0f }, { 0.0f, 0.0f });

		_scene.shapes.push_back(circle);
		_scene.shapes.push_back(bezierCurve);
	}
	inline void Teardown() override
	{

	}
	inline void Update(float deltaTime) override
	{
		if (pingpong >= 1.0f || pingpong <= 0.0f) incr = -incr;
		pingpong = std::min(1.0f, std::max(0.0f, pingpong + incr * deltaTime));

		circle->center.x = 100 + 100.0f * pingpong;
		circle->center.y = 100 + 100.0f * pingpong;
		circle->radius = 50.0f * pingpong;
		bezierCurve->GetPoint(1).pos.x = 400.0f + 100.0f * pingpong;
	}

	inline void Render() override
	{
		_recorder.Clear();
		_scene.Draw(_recorder);
		_renderer.BeginFrame(_renderSpecs);
		_renderer.Submit(_recorder.GetCommandBuffer());
		_renderer.EndFrame();

	}

	inline void SetRenderSpecs(const Core::IRenderer::RenderTargetSpecs& specs) override
	{
		_renderSpecs = specs;
	}

	inline const  Core::IRenderer::RenderTargetSpecs& GetRenderSpecs() const override
	{
		return _renderSpecs;
	}
};