#pragma once

#include <variant>
#include <string>
#include <vector>
#include <array>
#include <iostream> // Added for std::ostream

#include <cereal/archives/binary.hpp>

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

struct String64 {

    char data[64];
    String64() { data[0] = '\0'; }
    String64(const std::string& s) { strncpy_s(data, s.c_str(), 63); data[63] = '\0'; }
    operator std::string() const { return std::string(data);}
	
    String64& operator=(const char* s) { strncpy_s(data, s, 63); data[63] = '\0'; return *this; }
    std::string to_string() const { return std::string(data); }

    bool empty() const { return data[0] == '\0'; }
    size_t length() const { return std::strlen(data); }


    template<class Archive>
    void serialize(Archive& ar) { ar(data); }
};

// Add stream operator for String64 to support logging
inline std::ostream& operator<<(std::ostream& os, const String64& str) {
    os << str.data;
    return os;
}

// ----- Editor Value type (extend as needed)
using AppValueTypes = std::variant<bool, int, float, double, String64, Vec3>;
