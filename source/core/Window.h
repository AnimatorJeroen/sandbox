#pragma once
#include "event/EventBus.h"

namespace Core
{
	class Window
	{
	public:
		int Width;
		int Height;
		Window(int width, int height, const char* title, EventBus& eventBus);
		~Window();
		bool ShouldClose() const;
		void SwapBuffers() const;
		void GetFramebufferSize(int& width, int& height) const;
		void PollEvents() const;
		inline void* GetHandle() const { return _glfwWindow; }
	private:
		void* _glfwWindow;
		EventBus& _eventBus;
	};
}