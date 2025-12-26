#pragma once

namespace Core
{
	class Window
	{
	public:
		int Width;
		int Height;
		Window(int width, int height, const char* title);
		~Window();
		bool ShouldClose() const;
		void SwapBuffers() const;
		void PollEvents() const;
		inline void* GetHandle() const { return _glfwWindow; }
	private:
		void* _glfwWindow;
	};
}