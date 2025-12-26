#pragma once

namespace Core
{
	class IApplicationLayer
	{
	public:
		virtual void OnUpdate(const float deltaTime) = 0;
		virtual void OnRender() = 0;
	};
}