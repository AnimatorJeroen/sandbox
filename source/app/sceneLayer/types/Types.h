#pragma once

#include <variant>
#include <string>
#include <vector>
#include <array>
#include <iostream> // Added for std::ostream

#include "core/UUID.h"
#include <glm/glm.hpp>

using vec2 = glm::vec2;
using vec3 = glm::vec3;
using mat4 = glm::mat4;

// ----- Minimal data types
struct String64 {

    char data[64];
    String64() { data[0] = '\0'; }
    String64(const std::string& s) {
#ifdef PLATFORM_WASM
        strncpy(data, s.c_str(), 63);
#else
        strncpy_s(data, s.c_str(), 63);
#endif
        data[63] = '\0';
    }
    operator std::string() const { return std::string(data);}

    String64& operator=(const char* s) {
#ifdef PLATFORM_WASM
        strncpy(data, s, 63);
#else
        strncpy_s(data, s, 63);
#endif
        data[63] = '\0';
        return *this;
    }
    std::string to_string() const { return std::string(data); }

    bool empty() const { return data[0] == '\0'; }
    size_t length() const { return std::strlen(data); }
};

// Add stream operator for String64 to support logging
inline std::ostream& operator<<(std::ostream& os, const String64& str) {
    os << str.data;
    return os;
}

// ----- Editor Value type (extend as needed)
using AppFieldTypes = std::variant<bool, int, float, double, uint32_t, String64, vec2, vec3, mat4>;

















































