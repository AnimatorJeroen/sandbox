#pragma once

#include <variant>
#include <string>
#include <vector>
#include <array>

// ----- Minimal data types
struct Vec3 { 
    float x{}, y{}, z{}; 
};

// NEW: Nested structure for testing array inside array
struct Matrix2x3 {
    std::array<std::array<float, 3>, 2> data = { { {0.f, 0.f, 1.f}, {0.f, 0.f, 1.f} } };
};

struct Transform { 
    Vec3 Position{}; 
    Vec3 Scale{ 1.f, 1.f, 1.f };
    std::vector<float> Weights{}; // Added: vector field for demonstrating index access
    Matrix2x3 Matrix{};           // nested arrays for testing
};

struct GameObject { 
    Transform transform{}; 
};

// ----- Editor Value type (extend as needed)
using AppValueTypes = std::variant<bool, int, float, double, std::string, Vec3>;
