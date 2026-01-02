#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>
#include <entt/entt.hpp>
#include "Undo.h"
#include "app/sceneLayer/Scene.h"

namespace Core {

struct PathToken {
    std::string name;
    int index = -1;
};

inline std::vector<PathToken> Tokenize(std::string_view path) {
    std::vector<PathToken> out;
    size_t start = 0;
    while (start < path.size()) {
        size_t dot = path.find('.', start);
        std::string_view seg = path.substr(start, dot == std::string_view::npos ? path.size() - start : dot - start);
        PathToken tok;
        tok.name = std::string(seg);
        out.push_back(std::move(tok));
        if (dot == std::string_view::npos) break;
        start = dot + 1;
    }
    return out;
}

inline Value to_value_float(float f) { return Value{f}; }
inline float from_value_float(const Value &v) {
    if (auto pf = std::get_if<float>(&v)) return *pf;
    throw std::runtime_error("Expected float value");
}

class ChangeApplicator {
public:
    explicit ChangeApplicator(entt::registry &reg) : registry(reg) {}

    Value GetField(entt::entity e, entt::id_type compTypeId, std::string_view path) {
        auto tokens = Tokenize(path);
        if (tokens.empty()) throw std::runtime_error("Empty path");
        if (tokens.size() == 2 && tokens[0].name == "Scene") {
            Scene* scene = nullptr;
            if (registry.ctx().contains<Scene*>()) scene = registry.ctx().get<Scene*>();
            if (!scene) throw std::runtime_error("Scene* not in registry context");
            const std::string &name = tokens[1].name;
            if (name == "sceneColor") return to_value_float(scene->sceneColor);
            throw std::runtime_error("No such Scene field: " + name);
        }
        if (compTypeId == entt::type_id<DummyComponent>().hash()) {
            auto *c = registry.try_get<DummyComponent>(e);
            if (!c) throw std::runtime_error("Missing DummyComponent");
            const std::string &name = tokens.back().name;
            if (name == "value") return to_value_float(c->value);
            if (name == "r") return to_value_float(c->r);
            if (name == "g") return to_value_float(c->g);
            if (name == "b") return to_value_float(c->b);
            if (name == "a") return to_value_float(c->a);
            throw std::runtime_error("No such field: " + name);
        }
        throw std::runtime_error("Unsupported component type or path");
    }

    void SetField(entt::entity e, entt::id_type compTypeId, std::string_view path, const Value &v) {
        auto tokens = Tokenize(path);
        if (tokens.empty()) throw std::runtime_error("Empty path");
        if (tokens.size() == 2 && tokens[0].name == "Scene") {
            Scene* scene = nullptr;
            if (registry.ctx().contains<Scene*>()) scene = registry.ctx().get<Scene*>();
            if (!scene) throw std::runtime_error("Scene* not in registry context");
            const std::string &name = tokens[1].name;
            float f = from_value_float(v);
            if (name == "sceneColor") { scene->sceneColor = f; return; }
            throw std::runtime_error("No such final Scene field: " + name);
        }
        if (compTypeId == entt::type_id<DummyComponent>().hash()) {
            auto *c = registry.try_get<DummyComponent>(e);
            if (!c) throw std::runtime_error("Missing DummyComponent");
            const std::string &name = tokens.back().name;
            float f = from_value_float(v);
            if (name == "value") { c->value = f; return; }
            if (name == "r") { c->r = f; return; }
            if (name == "g") { c->g = f; return; }
            if (name == "b") { c->b = f; return; }
            if (name == "a") { c->a = f; return; }
            throw std::runtime_error("No such final field: " + name);
        }
        throw std::runtime_error("Unsupported component type or path");
    }

private:
    entt::registry &registry;
};

} // namespace Core
