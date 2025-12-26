#include "Window.h"
#include <glew/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>


namespace Core
{
	Window::Window(int width, int height, const char* title)
		: Width(width), Height(height)
	{
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        _glfwWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if (!_glfwWindow) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return;
        }
        glfwMakeContextCurrent((GLFWwindow*)_glfwWindow);
        glfwSwapInterval(1); // Enable vsync
	}
    Window::~Window()
    {
		glfwDestroyWindow((GLFWwindow*)_glfwWindow);
    }
    bool Window::ShouldClose() const
    {
        return glfwWindowShouldClose((GLFWwindow*)_glfwWindow);
    }
    void Window::PollEvents() const
    {
        glfwPollEvents();
    }
    void Window::SwapBuffers() const
    {
        glfwSwapBuffers((GLFWwindow*)_glfwWindow);
	}
}