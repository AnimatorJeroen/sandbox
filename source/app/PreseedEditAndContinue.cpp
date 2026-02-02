// Explicit Template Instantiations for Edit and Continue
// Only compiled in DEBUG builds with optimizations disabled

#include "pch.h"

#ifdef _DEBUG

// Disable ALL optimizations to prevent inlining
#pragma optimize("", off)

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <entt/entt.hpp>
#include "core/Logger.h"
#include "app/sceneLayer/components/Components.h"

// ============================================================================
// EXPLICIT TEMPLATE INSTANTIATIONS - GLM ONLY
// ============================================================================
// By explicitly instantiating these templates, we force the compiler to
// generate the code in this compilation unit. This makes Edit and Continue
// work better because the template code already exists in the binary.
// ============================================================================

namespace glm {
    // Vec3 operations
    template vec<3, float, defaultp> operator+(vec<3, float, defaultp> const&, vec<3, float, defaultp> const&);
    template vec<3, float, defaultp> operator-(vec<3, float, defaultp> const&, vec<3, float, defaultp> const&);
    template vec<3, float, defaultp> operator*(vec<3, float, defaultp> const&, float);
    template vec<3, float, defaultp> operator*(float, vec<3, float, defaultp> const&);
    template vec<3, float, defaultp> operator/(vec<3, float, defaultp> const&, float);
    
    // Vec3 functions
    template float dot(vec<3, float, defaultp> const&, vec<3, float, defaultp> const&);
    template vec<3, float, defaultp> cross(vec<3, float, defaultp> const&, vec<3, float, defaultp> const&);
    template vec<3, float, defaultp> normalize(vec<3, float, defaultp> const&);
    template float length(vec<3, float, defaultp> const&);
    
    // Mat4 operations
    template mat<4, 4, float, defaultp> operator*(mat<4, 4, float, defaultp> const&, mat<4, 4, float, defaultp> const&);
    
    // Transform functions
    template mat<4, 4, float, defaultp> translate(mat<4, 4, float, defaultp> const&, vec<3, float, defaultp> const&);
    template mat<4, 4, float, defaultp> rotate(mat<4, 4, float, defaultp> const&, float, vec<3, float, defaultp> const&);
    template mat<4, 4, float, defaultp> scale(mat<4, 4, float, defaultp> const&, vec<3, float, defaultp> const&);
    
    // Camera functions  
    template mat<4, 4, float, defaultp> lookAt(vec<3, float, defaultp> const&, vec<3, float, defaultp> const&, vec<3, float, defaultp> const&);
    template mat<4, 4, float, defaultp> perspective(float, float, float, float);
    
    // Quaternion functions
    template mat<4, 4, float, defaultp> toMat4(qua<float, defaultp> const&);
    template qua<float, defaultp> slerp(qua<float, defaultp> const&, qua<float, defaultp> const&, float);
    
    // Angle conversions
    template float radians(float);
    template float degrees(float);
    template vec<3, float, defaultp> radians(vec<3, float, defaultp> const&);
    template vec<3, float, defaultp> degrees(vec<3, float, defaultp> const&);
}

// ============================================================================
// RUNTIME PRESEED FUNCTIONS
// ============================================================================
// These functions are called at startup to ensure template code is exercised
// ============================================================================

namespace PreseedEditAndContinue {

    static volatile void* g_sink = nullptr;

    template<typename T>
    void Sink(const T& value) {
        g_sink = (void*)&value;
    }

    void PreseedGLM() {
        // GLM vector operations
        glm::vec3 v1(1.0f, 2.0f, 3.0f);
        glm::vec3 v2(4.0f, 5.0f, 6.0f);
        Sink(v1 + v2);
        Sink(v1 - v2);
        Sink(v1 * 2.0f);
        Sink(2.0f * v1);
        Sink(v1 / 2.0f);
        Sink(glm::dot(v1, v2));
        Sink(glm::cross(v1, v2));
        Sink(glm::normalize(v1));
        Sink(glm::length(v1));

        // GLM matrix operations
        glm::mat4 m = glm::mat4(1.0f);
        Sink(glm::translate(m, v1));
        Sink(glm::rotate(m, 1.0f, v1));
        Sink(glm::scale(m, v1));
        Sink(m * m);

        // Camera matrices
        Sink(glm::lookAt(v1, v2, glm::vec3(0, 1, 0)));
        Sink(glm::perspective(1.57f, 1.77f, 0.1f, 100.0f));

        // Quaternions
        glm::quat q1(1, 0, 0, 0);
        glm::quat q2(0, 1, 0, 0);
        Sink(glm::toMat4(q1));
        Sink(glm::slerp(q1, q2, 0.5f));

        // Angle conversions
        Sink(glm::radians(90.0f));
        Sink(glm::degrees(1.57f));
        Sink(glm::radians(v1));
        Sink(glm::degrees(v1));
    }

    void PreseedEnTT() {
        entt::registry reg;
        entt::entity e = reg.create();

        // Component operations - runtime preseed only (no explicit instantiation)
        reg.emplace<Core::UUID>(e);
        reg.emplace<Transform>(e);
        reg.emplace<NameComponent>(e);
        reg.emplace<CameraComponent>(e);
        reg.emplace<SceneData>(e);

        Sink(reg.get<Core::UUID>(e));
        Sink(reg.get<Transform>(e));
        Sink(reg.get<NameComponent>(e));
        Sink(reg.get<CameraComponent>(e));
        Sink(reg.get<SceneData>(e));

        Sink(reg.try_get<Core::UUID>(e));
        Sink(reg.try_get<Transform>(e));
        Sink(reg.try_get<NameComponent>(e));
        Sink(reg.try_get<CameraComponent>(e));
        Sink(reg.try_get<SceneData>(e));

        Sink(reg.all_of<Core::UUID>(e));
        Sink(reg.all_of<Transform>(e));

        // Views
        auto v1 = reg.view<Transform>();
        auto v2 = reg.view<Transform, NameComponent>();
        Sink(&v1);
        Sink(&v2);

        reg.destroy(e);
    }

    void PreseedLogger() {
        // Logger messages
        LOG_TRACE() << "Preseed";
        LOG_DEBUG() << "Preseed";
        LOG_INFO() << "Preseed";
        LOG_WARN() << "Preseed";
        LOG_ERROR() << "Preseed";
        LOG_CRITICAL() << "Preseed";

        // Different types
        int i = 42;
        float f = 3.14f;
        double d = 2.71;
        std::string s = "test";
        String64 s64("test64");
        glm::vec3 v(1.0f, 2.0f, 3.0f);
        
        LOG_INFO() << i;
        LOG_INFO() << f;
        LOG_INFO() << d;
        LOG_INFO() << s;
        LOG_INFO() << s64;
        LOG_INFO() << v.x;
    }

    void Preseed() {
        PreseedGLM();
        PreseedEnTT();
        PreseedLogger();
    }

} // namespace PreseedEditAndContinue

// Re-enable optimizations after this file
#pragma optimize("", on)

#endif // _DEBUG
