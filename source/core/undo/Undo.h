#pragma once
#include <vector>
#include <array>
#include <variant>
#include <type_traits>
#include <string>
#include <string_view>
#include <entt/entt.hpp>
#include <entt/meta/meta/meta.hpp>

namespace Core {

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
    entt::id_type compTypeId{};     // entt type id for the component (or Scene)
    FieldId fieldId{};              // optional hashed id of the field
    std::string path;               // optional: e.g. "Scene.sceneColor" or "Transform.Position.x"
    std::vector<entt::id_type> pathIds; // optional: meta id chain, preferred
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

struct PathToken { std::string name; int index = -1; };
inline std::vector<PathToken> Tokenize(std::string_view path) {
    std::vector<PathToken> out;
    size_t start = 0;
    while (start < path.size()) {
        size_t dot = path.find('.', start);
        std::string_view seg = path.substr(start, dot == std::string_view::npos ? path.size() - start : dot - start);
        PathToken tok; tok.name = std::string(seg);
        out.push_back(std::move(tok));
        if (dot == std::string_view::npos) break;
        start = dot + 1;
    }
    return out;
}

inline entt::meta_any to_meta_any(const Value& v) {
    if (auto pf = std::get_if<float>(&v)) return entt::meta_any{*pf};
    if (auto pv = std::get_if<Vec3>(&v)) return entt::meta_any{*pv};
    if (auto pc = std::get_if<Color>(&v)) return entt::meta_any{*pc};
    return {};
}

inline bool MetaSetByIds(entt::meta_any inst, const std::vector<entt::id_type>& ids, const Value& v) {
    if (!inst) return false;
    entt::meta_any cur = inst;
    if (ids.empty()) return false;
    // descend to parent of final field
    for (size_t i = 0; i < ids.size(); i++) {
        auto data = cur.type().data(ids[i]);
        if (!data) return false;
        cur = data.get(cur);
        if (!cur) return false;
    }
    auto final = cur.type().data(ids.back());
    if (!final) return false;
    return final.set(cur, to_meta_any(v));
}

inline bool MetaSetByPath(entt::meta_any inst, std::string_view path, const Value& v) {
    if (!inst) return false;
    auto tokens = Tokenize(path);
    if (tokens.empty()) return false;
    entt::meta_any cur = inst;
    for (size_t i = 0; i < tokens.size(); i++) {
        auto data = cur.type().data(entt::hashed_string{tokens[i].name.c_str()});
        if (!data) return false;
        cur = data.get(cur);
        if (!cur) return false;
    }
    auto last = cur.type().data(entt::hashed_string{tokens.back().name.c_str()});
    if (!last) return false;
    return last.set(cur, to_meta_any(v));
}

struct UndoableCommand {
    std::vector<Patch> patches;

    template<class Registry>
    void Apply(Registry& registry) const {
        for (const auto& p : patches) {
            // Scene path via meta
            if (!p.path.empty() && p.path.rfind("Scene", 0) == 0) {
                if (!registry.ctx().contains<class Scene*>()) continue;
                auto* scene = registry.ctx().get<class Scene*>();
                entt::meta_any cur{*scene};
                // try after "Scene." prefix (length 6 when including dot)
                if (p.path.size() > 6) {
                    MetaSetByPath(cur, p.path.substr(6), p.newValue);
                    continue;
                }
                // fallback to full path
                MetaSetByPath(cur, p.path, p.newValue);
                continue;
            }

            // Component: get component and set via meta
            entt::meta_type mt = entt::resolve(p.compTypeId);
            if (!mt) continue;
            auto storage = registry.storage(p.compTypeId);
            if (!storage) continue;
            void* comp_ptr = storage->value(p.entity);
            if (!comp_ptr) continue;
            entt::meta_any inst = mt.from_void(comp_ptr);

            // Prefer id chain when available
            if (!p.pathIds.empty()) {
                MetaSetByIds(inst, p.pathIds, p.newValue);
                continue;
            }

            // Fallback to string path
            if (!p.path.empty()) {
                MetaSetByPath(inst, p.path, p.newValue);
                continue;
            }

            // Fallback to single field id
            if (p.fieldId) {
                auto data = inst.type().data(p.fieldId);
                if (data) {
                    data.set(inst, to_meta_any(p.newValue));
                }
            }
        }
    }

    template<class Registry>
    void Revert(Registry& registry) const {
        for (const auto& p : patches) {
            if (!p.path.empty() && p.path.rfind("Scene", 0) == 0) {
                if (!registry.ctx().contains<class Scene*>()) continue;
                auto* scene = registry.ctx().get<class Scene*>();
                entt::meta_any cur{*scene};
                if (p.path.size() > 6) {
                    MetaSetByPath(cur, p.path.substr(6), p.oldValue);
                    continue;
                }
                MetaSetByPath(cur, p.path, p.oldValue);
                continue;
            }

            entt::meta_type mt = entt::resolve(p.compTypeId);
            if (!mt) continue;
            auto storage = registry.storage(p.compTypeId);
            if (!storage) continue;
            void* comp_ptr = storage->value(p.entity);
            if (!comp_ptr) continue;
            entt::meta_any inst = mt.from_void(comp_ptr);

            if (!p.pathIds.empty()) {
                MetaSetByIds(inst, p.pathIds, p.oldValue);
                continue;
            }
            if (!p.path.empty()) {
                MetaSetByPath(inst, p.path, p.oldValue);
                continue;
            }
            if (p.fieldId) {
                auto data = inst.type().data(p.fieldId);
                if (data) {
                    data.set(inst, to_meta_any(p.oldValue));
                }
            }
        }
    }
};

} // namespace Core
