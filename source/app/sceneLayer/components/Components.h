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
    Vec3 Position{};
    Vec3 Scale{ 1.f, 1.f, 1.f };
    std::vector<float> Weights{}; // Added: vector field for demonstrating index access
    Matrix2x3 Matrix{};           // nested arrays for testing
};


// Define all component types here as a tuple
// Add ALL your component types to this list, to be registered with the Applicator
using AppComponentTypes = std::tuple<
    Core::UUID,
    Transform,
    NameComponent
>;