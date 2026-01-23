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
        
        GLuint m_ShaderProgram = 0;
        GLuint m_VertexShader = 0;
        GLuint m_FragmentShader = 0;
        
        GLuint m_LineVAO = 0;
        GLuint m_LineVBO = 0;
        
        GLint m_UniformColor = -1;
        GLint m_UniformProjection = -1;
        
        RenderTargetSpecs m_CurrentTarget;
        
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
