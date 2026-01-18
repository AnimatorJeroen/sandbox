#pragma once

namespace Core
{
	class EventBus;

	class Window
	{
	public:
		int Width;
		int Height;
		Window(int width, int height, const char* title, EventBus& eventBus);
		~Window();
		void SetCallBacks() const;
		bool ShouldClose() const;
		void SwapBuffers() const;
		void GetFramebufferSize(int& width, int& height) const;
		void PollEvents() const;
		inline void* GetHandle() const { return _glfwWindow; }
		void* GetNativeWindowHandle() const; // Get platform-specific window handle (HWND on Windows)
	private:
		void* _glfwWindow;
		EventBus* _eventBus;
	};
}