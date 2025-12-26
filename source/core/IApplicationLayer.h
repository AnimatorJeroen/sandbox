#pragma once
#include "LayerContext.h"
namespace Core
{
	class IApplicationLayer
	{
	public:
		virtual void OnUpdate(const float deltaTime) = 0;
		virtual void OnRender() = 0;
	protected:
		inline explicit IApplicationLayer(Core::LayerContext& ctx) {}
	};
}