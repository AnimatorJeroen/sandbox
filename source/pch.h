#pragma once

#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <stack>
#include <string>
#include <optional>
#include <memory>
#include <functional>
#include <chrono>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <cstdint>
#include <array>

#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/polymorphic.hpp>

#include <entt/entt.hpp>
#include <entt/meta/meta.hpp>

#include <glm/glm.hpp>
#include <imgui/imgui.h>

#include <glew/glew.h>
#include <GLFW/glfw3.h>

#include "core/Logger.h"
#include "core/UUID.h"
#include "core/Application.h"
#include "core/event/EventBus.h"
#include "core/renderer/DrawCommandRecorder.h"
#include "core/serializer/Serializer.h"
#include "core/reflection/Reflection.h"
#include "core/applicator/Applicator.h"

#include "app/sceneLayer/types/Types.h"
#include "app/sceneLayer/components/Components.h"
