#pragma once
#include "ITestScene.h"
#include "scene/Scene.h"
#include "renderer/Renderer_ImGui.h"

class TestScene1 : public ITestScene
{
private:
	Scene _scene;
	DrawCommandRecorder _recorder;
	Renderer_ImGui renderer;

	float pingpong = 0.0f;
	float incr = 0.5f;


	std::shared_ptr<Circle> circle = std::make_shared<Circle>(
		glm::vec2{100.0f, 100.0f},  // center
		50.0f,                        // radius
		true,                         // filled
		ColorRGBA{ 0.0f, 0.0f, 1.0f, 1.0f },  // color
		32,                           // num_segments
		1.0f                          // thickness
	);

	std::shared_ptr<BezierCurve> bezierCurve = std::make_shared<BezierCurve>();

public:
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
		_recorder.Clear();

		pingpong += incr * deltaTime;
		if (pingpong >= 1.0f || pingpong <= 0.0f) incr = -incr;

		circle->center.x = 100 + 100.0f * pingpong;
		circle->center.y = 100 + 100.0f * pingpong;
		circle->radius = 50.0f * pingpong;
		bezierCurve->GetPoint(1).pos.x = 400.0f + 100.0f * pingpong;

		_scene.Draw(_recorder);

		int wWidth = 800;
		int wHeight = 600;
		renderer.BeginFrame({ (unsigned int)wWidth, (unsigned int)wHeight });
		renderer.Submit(_recorder.GetCommandBuffer());
		renderer.EndFrame();

	}
};