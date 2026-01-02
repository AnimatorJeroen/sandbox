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
        throw std::runtime_error("Unsupported component type or path");
    }

    void SetField(entt::entity e, entt::id_type compTypeId, std::string_view path, const Value &v) {
        if (!registry) throw std::runtime_error("ChangeApplicator: registry context not set");
        auto &reg = *registry;
        Value old = GetField(e, compTypeId, path);
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
            UndoableCommand cmd;
            Patch p;
            p.entity = e;
            p.compTypeId = entt::type_id<Scene>().hash();
            p.fieldId = entt::hashed_string(name.c_str()).value();
            p.path = std::string(path);
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
