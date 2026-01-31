#pragma once
#include "pch.h"
#include "app/sceneLayer/types/Types.h"

// Name component for entities
struct NameComponent {
    String64 name;

    NameComponent() = default;
    NameComponent(const std::string& n) : name(n) {}

    template<class Archive>
    void serialize(Archive& ar) {
        ar(name);
    }
};

struct Transform {
    vec4 Position{};
    vec4 Scale{ 1.f, 1.f, 1.f, 1.f };
	mat4 Matrix{ 1.0f };

    template<class Archive>
    void serialize(Archive& ar) {
        ar(Position, Scale);
    }
};


// Define all component types here as a tuple
// Add ALL your component types to this list, to be registered with the Applicator
using AppComponentTypes = std::tuple<
    Core::UUID,
    Transform,
    NameComponent
>;