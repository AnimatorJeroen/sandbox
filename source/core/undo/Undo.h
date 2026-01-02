#pragma once
#include <vector>
#include <array>
#include <variant>
#include <type_traits>
#include <entt/entt.hpp>

namespace Core {

// Forward declare component types only if available; otherwise keep generic.
// struct Transform;
// struct Renderer;

// Minimal value carrier for undo patches. Extend as needed.
struct Vec3 { float x{}, y{}, z{}; };
struct Color { float r{}, g{}, b{}, a{}; };

using Value = std::variant<std::monostate, float, Vec3, Color>;
using FieldId = entt::id_type;

// Common hashed string IDs for fields
inline constexpr FieldId F_Position      = entt::hashed_string("Position").value();
inline constexpr FieldId F_Scale         = entt::hashed_string("Scale").value();
inline constexpr FieldId F_Position_x    = entt::hashed_string("Position.x").value();
inline constexpr FieldId F_Position_y    = entt::hashed_string("Position.y").value();
inline constexpr FieldId F_Position_z    = entt::hashed_string("Position.z").value();
inline constexpr FieldId F_BaseColor     = entt::hashed_string("BaseColor").value();

struct Patch {
    entt::entity entity{entt::null};
    entt::id_type compTypeId{}; // entt type id for the component
    FieldId fieldId{};          // compile-time hash id
    Value oldValue{};
    Value newValue{};
};

// Accessor table for components
template<class C>
struct Accessor {
    using Getter = Value(*)(C&);
    using Setter = void(*)(C&, const Value&);
    FieldId id{};
    Getter get{};
    Setter set{};
};

// Default: empty table
template<class C>
constexpr auto build_accessor_table() {
    return std::array<Accessor<C>, 0>{};
}

// Lookup is O(N) over a tiny array; optionally sort and binary search
template<class C, std::size_t N>
constexpr Accessor<C> find_accessor(FieldId id, const std::array<Accessor<C>, N>& arr) {
    for (const auto& a : arr) {
        if (a.id == id) return a;
    }
    return Accessor<C>{};
}

struct UndoableCommand {
    std::vector<Patch> patches;

    template<class Registry>
    void Apply(Registry& /*registry*/) const {
        // No-op by default; provide specializations per component type elsewhere.
    }

    template<class Registry>
    void Revert(Registry& /*registry*/) const {
        // No-op by default; provide specializations per component type elsewhere.
    }
};

} // namespace Core
