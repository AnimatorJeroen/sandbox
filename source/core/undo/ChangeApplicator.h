#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>
#include <entt/entt.hpp>
#include "Undo.h"
#include "UndoManager.h"
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
    explicit ChangeApplicator(UndoManager &um) : undoManager(um) {}

    void SetContext(entt::registry &reg) { registry = &reg; }

    Value GetField(entt::entity e, entt::id_type compTypeId, std::string_view path) {
        if (!registry) throw std::runtime_error("ChangeApplicator: registry context not set");
        auto &reg = *registry;
        auto tokens = Tokenize(path);
        if (tokens.empty()) throw std::runtime_error("Empty path");
        if (tokens.size() == 2 && tokens[0].name == "Scene") {
            Scene* scene = nullptr;
            if (reg.ctx().contains<Scene*>()) scene = reg.ctx().get<Scene*>();
            if (!scene) throw std::runtime_error("Scene* not in registry context");
            const std::string &name = tokens[1].name;
            if (name == "sceneColor") return to_value_float(scene->sceneColor);
            throw std::runtime_error("No such Scene field: " + name);
        }
        if (compTypeId == entt::type_id<DummyComponent>().hash()) {
            auto *c = reg.try_get<DummyComponent>(e);
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
        if (!registry) throw std::runtime_error("ChangeApplicator: registry context not set");
        auto &reg = *registry;
        // Capture old value
        Value old = GetField(e, compTypeId, path);
        // Perform the write
        auto tokens = Tokenize(path);
        if (tokens.empty()) throw std::runtime_error("Empty path");
        if (tokens.size() == 2 && tokens[0].name == "Scene") {
            Scene* scene = nullptr;
            if (reg.ctx().contains<Scene*>()) scene = reg.ctx().get<Scene*>();
            if (!scene) throw std::runtime_error("Scene* not in registry context");
            const std::string &name = tokens[1].name;
            float f = from_value_float(v);
            if (name == "sceneColor") { scene->sceneColor = f; }
            else { throw std::runtime_error("No such final Scene field: " + name); }
            // Push undo command
            UndoableCommand cmd;
            Patch p;
            p.entity = e;
            p.compTypeId = entt::type_id<Scene>().hash();
            p.fieldId = entt::hashed_string(name.c_str()).value();
            p.oldValue = old;
            p.newValue = v;
            cmd.patches.push_back(p);
            undoManager.Push(cmd);
            return;
        }
        if (compTypeId == entt::type_id<DummyComponent>().hash()) {
            auto *c = reg.try_get<DummyComponent>(e);
            if (!c) throw std::runtime_error("Missing DummyComponent");
            const std::string &name = tokens.back().name;
            float f = from_value_float(v);
            if (name == "value") { c->value = f; }
            else if (name == "r") { c->r = f; }
            else if (name == "g") { c->g = f; }
            else if (name == "b") { c->b = f; }
            else if (name == "a") { c->a = f; }
            else { throw std::runtime_error("No such final field: " + name); }
            UndoableCommand cmd;
            Patch p;
            p.entity = e;
            p.compTypeId = entt::type_id<DummyComponent>().hash();
            p.fieldId = entt::hashed_string(name.c_str()).value();
            p.oldValue = old;
            p.newValue = v;
            cmd.patches.push_back(p);
            undoManager.Push(cmd);
            return;
        }
        throw std::runtime_error("Unsupported component type or path");
    }

private:
    entt::registry *registry{nullptr};
    UndoManager &undoManager;
};

} // namespace Core
