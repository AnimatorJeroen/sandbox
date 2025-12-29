#pragma once
#include "core/IApplicationLayer.h"
#include "core/event/EventBus.h"
#include "core/event/MouseEvent.h"
#include "core/event/KeyEvent.h"
#include "core/event/ApplicationEvent.h"
#include "app/event/SceneEvent.h"

class LoggingApplicationLayer : public Core::IApplicationLayer
{
public:
	explicit LoggingApplicationLayer(Core::LayerContext& ctx);
	~LoggingApplicationLayer() = default;
	void OnUpdate(const float deltaTime) override;
	void OnRender() override;

private:
	Core::EventBus& _eventBus;
};
