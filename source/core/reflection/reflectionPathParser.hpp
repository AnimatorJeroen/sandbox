#pragma once

#include "../vendor/include/entt/entt.hpp"
#include <array>
#include <string_view>

// ===========================================================================
// PATH PARSING LIBRARY (C++20)
// ===========================================================================
//
// This header provides both compile-time and runtime path parsing utilities
// for navigating reflection paths like "Transform.Position.x" or "Transform.Weights[1].data[0]".
//
// KEY FEATURES:
// - COMPILE-TIME: TRUE compile-time string parsing using C++20 consteval
// - RUNTIME: Runtime string parsing from std::string or const char*
// - Fixed-size std::array (max 16 elements)
// - Sentinel-terminated (default PathElement marks end, like null terminator)
// - Component type can be extracted from the first path element
//
// ===========================================================================

namespace Core {


    namespace reflection {

        // Maximum path elements (excluding component type)
        inline constexpr size_t MAX_PATH_ELEMENTS = 16;

        // Path type: just a fixed-size array, sentinel-terminated
        using Path = std::array<PathElement, MAX_PATH_ELEMENTS>;

        // Helper: Check if PathElement is sentinel (default-constructed)
        inline constexpr bool IsSentinel(const PathElement& elem) {
            // Default-constructed variant holds first alternative (entt::id_type) with value 0
            return std::holds_alternative<entt::id_type>(elem) && std::get<entt::id_type>(elem) == 0;
        }

        // Helper: Get length of path (find sentinel)
        inline constexpr size_t PathLength(const Path& path) {
            for (size_t i = 0; i < MAX_PATH_ELEMENTS; ++i) {
                if (IsSentinel(path[i])) {
                    return i;
                }
            }
            return MAX_PATH_ELEMENTS;
        }

        // ===== FULL PATH WITH COMPONENT TYPE =====
        // Stores both component type hash and property path separately
        struct FullPath {
            entt::id_type componentType{};  // Hash of component type name
            Path propertyPath{};             // Property path within component

            constexpr FullPath() = default;
            constexpr FullPath(entt::id_type comp, const Path& path)
                : componentType(comp), propertyPath(path) {
            }
        };

        // ===== COMPILE-TIME PATH PARSING (C++20) =====

        // ===== COMPILE-TIME PATH PARSER =====
        // This struct parses path strings at compile-time using consteval
        // Now extracts component type from first element

        template<size_t N>
        struct CTPath {
            static constexpr size_t max_elements = MAX_PATH_ELEMENTS;
            entt::id_type componentType{};
            PathElement elements[max_elements]{};  // Default-initialized (all sentinels)
            size_t count = 0;

            // consteval forces compile-time evaluation
            consteval CTPath(const char(&str)[N]) {
                const char* p = str;
                bool firstElement = true;

                while (*p && count < max_elements) {
                    // Skip dots
                    if (*p == '.') {
                        ++p;
                        continue;
                    }

                    // Check if this is an index [N]
                    if (*p == '[') {
                        ++p;
                        size_t idx = 0;
                        while (*p >= '0' && *p <= '9') {
                            idx = idx * 10 + (*p - '0');
                            ++p;
                        }
                        if (*p == ']') ++p;
                        elements[count++] = PathElement{ idx };
                        firstElement = false;
                    }
                    // Otherwise it's a field name
                    else {
                        const char* start = p;
                        while (*p && *p != '.' && *p != '[') {
                            ++p;
                        }

                        // Extract field name
                        char field_name[64]{};
                        size_t len = 0;
                        for (const char* s = start; s < p && len < 63; ++s, ++len) {
                            field_name[len] = *s;
                        }
                        field_name[len] = '\0';

                        // Hash at compile-time using entt::hashed_string
                        auto hash = entt::hashed_string{ field_name }.value();

                        if (firstElement) {
                            // First element is the component type
                            componentType = hash;
                            firstElement = false;
                        }
                        else {
                            // Subsequent elements are property path
                            elements[count++] = PathElement{ hash };
                        }
                    }
                }
                // Remaining elements are already default-initialized (sentinels)
            }

            // Convert to FullPath
            constexpr FullPath to_full_path() const {
                Path result{};
                for (size_t i = 0; i < max_elements; ++i) {
                    result[i] = elements[i];
                }
                return FullPath{ componentType, result };
            }

            // Legacy: Convert to Path (just copy the property path array)
            constexpr Path to_path() const {
                Path result{};
                for (size_t i = 0; i < max_elements; ++i) {
                    result[i] = elements[i];
                }
                return result;
            }
        };

        // ===== RUNTIME PATH PARSING =====

        // Runtime path parser - parses strings at runtime
        // Returns FullPath with component type and property path separated
        inline FullPath ParseFullPathRuntime(std::string_view path_str) {
            entt::id_type componentType{};
            Path result{};  // Default-initialized (all sentinels)
            size_t count = 0;
            bool firstElement = true;

            const char* p = path_str.data();
            const char* end = p + path_str.size();

            while (p < end && *p && (firstElement || count < MAX_PATH_ELEMENTS)) {
                // Skip dots
                if (*p == '.') {
                    ++p;
                    continue;
                }

                // Check if this is an index [N]
                if (*p == '[') {
                    ++p;
                    size_t idx = 0;
                    while (p < end && *p >= '0' && *p <= '9') {
                        idx = idx * 10 + (*p - '0');
                        ++p;
                    }
                    if (p < end && *p == ']') ++p;
                    result[count++] = PathElement{ idx };
                    firstElement = false;
                }
                // Otherwise it's a field name
                else {
                    const char* start = p;
                    while (p < end && *p && *p != '.' && *p != '[') {
                        ++p;
                    }

                    // Create field name string
                    std::string field_name(start, p - start);

                    // Hash at runtime using entt::hashed_string
                    auto hash = entt::hashed_string{ field_name.c_str() }.value();

                    if (firstElement) {
                        // First element is the component type
                        componentType = hash;
                        firstElement = false;
                    }
                    else {
                        // Subsequent elements are property path
                        result[count++] = PathElement{ hash };
                    }
                }
            }

            // Remaining elements are already default-initialized (sentinels)
            return FullPath{ componentType, result };
        }

        // Legacy runtime path parser - parses strings at runtime
        // Returns the same Path type (sentinel-terminated fixed-size array)
        inline Path ParsePathRuntime(std::string_view path_str) {
            Path result{};  // Default-initialized (all sentinels)
            size_t count = 0;

            const char* p = path_str.data();
            const char* end = p + path_str.size();

            while (p < end && *p && count < MAX_PATH_ELEMENTS) {
                // Skip dots
                if (*p == '.') {
                    ++p;
                    continue;
                }

                // Check if this is an index [N]
                if (*p == '[') {
                    ++p;
                    size_t idx = 0;
                    while (p < end && *p >= '0' && *p <= '9') {
                        idx = idx * 10 + (*p - '0');
                        ++p;
                    }
                    if (p < end && *p == ']') ++p;
                    result[count++] = PathElement{ idx };
                }
                // Otherwise it's a field name
                else {
                    const char* start = p;
                    while (p < end && *p && *p != '.' && *p != '[') {
                        ++p;
                    }

                    // Create field name string
                    std::string field_name(start, p - start);

                    // Hash at runtime using entt::hashed_string
                    result[count++] = PathElement{ entt::hashed_string{field_name.c_str()}.value() };
                }
            }

            // Remaining elements are already default-initialized (sentinels)
            return result;
        }

        // Overload for const char* convenience
        inline FullPath ParseFullPathRuntime(const char* path_str) {
            return ParseFullPathRuntime(std::string_view{ path_str });
        }

        // Overload for std::string convenience
        inline FullPath ParseFullPathRuntime(const std::string& path_str) {
            return ParseFullPathRuntime(std::string_view{ path_str });
        }

        // Legacy overloads
        inline Path ParsePathRuntime(const char* path_str) {
            return ParsePathRuntime(std::string_view{ path_str });
        }

        inline Path ParsePathRuntime(const std::string& path_str) {
            return ParsePathRuntime(std::string_view{ path_str });
        }

    } // namespace reflection

    // ===========================================================================
    // PATH PARSING MACROS
    // ===========================================================================

    // ===========================================================================
    // COMPILE-TIME PATH MACROS (C++20)
    // ===========================================================================

    // COMPILE-TIME FULL PATH PARSING with component type extraction
    // Parses strings like "Transform.Position.x" at compile-time with ZERO runtime cost!
    // Returns FullPath with componentType and propertyPath separated
    //
    // Usage: PATH_FULL_CONSTEXPR("Transform.Position.x")
    //        PATH_FULL_CONSTEXPR("Transform.Weights[1]")
    //        PATH_FULL_CONSTEXPR("Transform.Matrix.data[1][2]")
#define PATH_FULL_CONSTEXPR(path_str) \
    ([]() constexpr { \
        constexpr Core::reflection::CTPath path{path_str}; \
        return path.to_full_path(); \
    }())

// COMPILE-TIME PATH PARSING using fixed-size sentinel-terminated array
// Parses strings like "Weights[1].x" at compile-time with ZERO runtime cost!
// Returns std::array<PathElement, 16> (sentinel-terminated like C strings)
//
// Usage: PATH_CONSTEXPR("Position.x")
//        PATH_CONSTEXPR("Weights[1]")
//        PATH_CONSTEXPR("Matrix.data[1][2]")
#define PATH_CONSTEXPR(path_str) \
    ([]() constexpr { \
        constexpr Core::reflection::CTPath path{path_str}; \
        return path.to_path(); \
    }())

// ===========================================================================
// RUNTIME PATH PARSING
// ===========================================================================

// RUNTIME FULL PATH PARSING - parses at runtime from string variables
// Returns FullPath with componentType and propertyPath separated
//
// Usage: PATH_FULL_RUNTIME(myString)
//        PATH_FULL_RUNTIME("Transform.Position.x")
//        PATH_FULL_RUNTIME(std::string("Transform.Weights[1]"))
#define PATH_FULL_RUNTIME(path_str) \
    (Core::reflection::ParseFullPathRuntime(path_str))

// RUNTIME PATH PARSING - parses at runtime from string variables
// Returns std::array<PathElement, 16> (sentinel-terminated like C strings)
//
// Usage: PATH_RUNTIME(myString)
//        PATH_RUNTIME("Position.x")  // Also works with literals
//        PATH_RUNTIME(std::string("Weights[1]"))
#define PATH_RUNTIME(path_str) \
    (Core::reflection::ParsePathRuntime(path_str))

}