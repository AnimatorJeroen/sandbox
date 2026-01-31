#pragma once

#include <variant>
#include <string>
#include <vector>
#include <array>
#include <iostream> // Added for std::ostream

#include <cereal/archives/binary.hpp>
#include "core/UUID.h"
#include <glm/glm.hpp>

using vec4 = glm::vec4;
using mat4 = glm::mat4;

namespace cereal {
    template<class Archive>
    void serialize(Archive& ar, glm::vec4& v) {
        ar(v.x, v.y, v.z, v.w);
    }
}

// ----- Minimal data types
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
using AppFieldTypes = std::variant<bool, int, float, double, String64, vec4, mat4>;




