#include "glew/glew.h"
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <iostream>
#include "renderer/Renderer_ImGui.h"
#include "renderer/DrawCommandRecorder.h"
#include "shape/Circle.h"
#include "shape/BezierCurve.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Sandbox", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	Renderer_ImGui renderer;

    auto rec = DrawCommandRecorder();
	float pingpong = 0.0f;
	float incr = 0.5f;

    float deltaTime = 0.0f;
    float lastFrameTime = 0.0f;
    float currentFrameTime = 0.0f;

	int wWidth, wHeight;

	Circle circle{
        {100.0f, 100.0f},  // center
        50.0f,                        // radius
        true,                         // filled
        { 0.0f, 0.0f, 1.0f, 1.0f },  // color
        32,                           // num_segments
        1.0f                          // thickness
    };

    BezierCurve bezierCurve;
	bezierCurve.AddPoint({ 300.0f, 300.0f }, { -50.0f, 0.0f }, { 50.0f, 0.0f });
	bezierCurve.AddPoint({ 400.0f, 400.0f }, { -50.0f, 0.0f }, { 50.0f, 0.0f });
	bezierCurve.AddPoint({ 500.0f, 300.0f }, { -50.0f, 0.0f }, { 0.0f, 0.0f });

    //application mainloop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        currentFrameTime = static_cast<float>(glfwGetTime());
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        pingpong += incr * deltaTime;
		if (pingpong >= 1.0f || pingpong <= 0.0f) incr = -incr;

        rec.Clear();
		rec.Line({ 100 * pingpong, 100 }, { 200, 200 }, 5.0f, { 1.0f, 0.0f, 0.0f, 1.0f });
		rec.Line({ 200, 100 }, { 100, 200 }, 3.0f, { 0.0f, 1.0f, 0.0f, 1.0f });
		
		circle.center.x = 100 + 100.0f * pingpong;
		circle.center.y = 100 + 100.0f * pingpong;
        circle.radius = 50.0f * pingpong;
		bezierCurve.GetPoint(1).pos.x = 400.0f + 100.0f * pingpong;

		circle.Draw(rec);
		bezierCurve.Draw(rec);


        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Sandbox Controls");
        ImGui::Text("Adjust clear color:");
        ImGui::ColorEdit3("Background Color", (float*)&clear_color);
        ImGui::End();

        glfwGetWindowSize(window, &wWidth, &wHeight);
		renderer.BeginFrame({ (unsigned int)wWidth, (unsigned int)wHeight });
        renderer.Submit(rec.GetCommandBuffer());
		renderer.EndFrame();

        ImGui::Render();
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}