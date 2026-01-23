#pragma once

#include "IRenderer.h"
#include "IMesh.h"
#include <memory>
#include <glew/glew.h>

namespace Core {

    class OpenGLMesh : public IMesh {
    public:
        OpenGLMesh();
        ~OpenGLMesh() override;

        void SetVertices(const std::vector<Vertex>& vertices) override;
        void SetIndices(const std::vector<uint32_t>& indices) override;
        
        const std::vector<Vertex>& GetVertices() const override { return m_Vertices; }
        const std::vector<uint32_t>& GetIndices() const override { return m_Indices; }
        
        void Upload() override;
        void Bind() const override;
        void Unbind() const override;

    private:
        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;
        
        GLuint m_VAO = 0;
        GLuint m_VBO = 0;
        GLuint m_EBO = 0;
    };

    class Renderer_OpenGL : public IRenderer {
    public:
        Renderer_OpenGL();
        ~Renderer_OpenGL() override;

        void BeginFrame(const RenderTargetSpecs& target) override;
        void Submit(const DrawCommandBuffer& cmdBuf) override;
        void EndFrame() override;

        // Mesh rendering support
        std::shared_ptr<IMesh> CreateMesh() override;
        void DrawMesh(const std::shared_ptr<IMesh>& mesh) override;
        void DrawMesh(const std::shared_ptr<IMesh>& mesh, const ColorRGBA& color);

    private:
        void InitializeShaders();
        void CleanupShaders();
        void SetupLineRendering();
        void SetupCubeRendering();
        void CreateDefaultCube();
        
        GLuint m_ShaderProgram = 0;
        GLuint m_VertexShader = 0;
        GLuint m_FragmentShader = 0;
        
        // 3D cube shader
        GLuint m_CubeShaderProgram = 0;
        GLuint m_CubeVertexShader = 0;
        GLuint m_CubeFragmentShader = 0;
        GLint m_CubeUniformMVP = -1;
        GLint m_CubeUniformColor = -1;
        
        GLuint m_LineVAO = 0;
        GLuint m_LineVBO = 0;
        
        // Default cube mesh
        GLuint m_CubeVAO = 0;
        GLuint m_CubeVBO = 0;
        GLuint m_CubeEBO = 0;
        uint32_t m_CubeIndexCount = 0;
        
        GLint m_UniformColor = -1;
        GLint m_UniformProjection = -1;
        
        RenderTargetSpecs m_CurrentTarget;
        
        // 3D Camera matrices
        glm::mat4 m_ViewMatrix;
        glm::mat4 m_ProjectionMatrix;
        
        // Polygon state tracking
        std::vector<Vertex> m_PolygonVertices;
        ColorRGBA m_PolygonColor = {1.0f, 1.0f, 1.0f, 1.0f};
        float m_PolygonThickness = 1.0f;
        bool m_PolygonFilled = false;
        
        // Cube instancing - collect all cubes and render in batch
        struct CubeInstance {
            glm::mat4 transform;
            ColorRGBA color;
        };
        std::vector<CubeInstance> m_CubeInstances;
        
        // Saved OpenGL state for restoration
        GLuint m_SavedShaderProgram = 0;
        GLuint m_SavedVAO = 0;
        GLboolean m_SavedDepthTest = GL_FALSE;
        GLboolean m_SavedCullFace = GL_FALSE;
        GLboolean m_SavedBlend = GL_FALSE;
        GLenum m_SavedBlendSrcAlpha = GL_ONE;
        GLenum m_SavedBlendDstAlpha = GL_ZERO;
    };

}
