#include "pch.h"
#include "Renderer_OpenGL.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Core {

    // Vertex shader source
    static const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoord;
        layout (location = 2) in vec4 aColor;
        
        uniform mat4 uProjection;
        
        out vec2 TexCoord;
        out vec4 Color;
        
        void main()
        {
            gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
            TexCoord = aTexCoord;
            Color = aColor;
        }
    )";

    // Fragment shader source
    static const char* fragmentShaderSource = R"(
        #version 330 core
        in vec2 TexCoord;
        in vec4 Color;
        
        out vec4 FragColor;
        
        void main()
        {
            FragColor = Color;
        }
    )";

    // ========== OpenGLMesh Implementation ==========

    OpenGLMesh::OpenGLMesh() {
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);
    }

    OpenGLMesh::~OpenGLMesh() {
        if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
        if (m_VBO) glDeleteBuffers(1, &m_VBO);
        if (m_EBO) glDeleteBuffers(1, &m_EBO);
    }

    void OpenGLMesh::SetVertices(const std::vector<Vertex>& vertices) {
        m_Vertices = vertices;
    }

    void OpenGLMesh::SetIndices(const std::vector<uint32_t>& indices) {
        m_Indices = indices;
    }

    void OpenGLMesh::Upload() {
        glBindVertexArray(m_VAO);

        // Upload vertex data
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), m_Vertices.data(), GL_STATIC_DRAW);

        // Upload index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(uint32_t), m_Indices.data(), GL_STATIC_DRAW);

        // Set vertex attributes
        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        
        // TexCoord
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
        
        // Color
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

        glBindVertexArray(0);
    }

    void OpenGLMesh::Bind() const {
        glBindVertexArray(m_VAO);
    }

    void OpenGLMesh::Unbind() const {
        glBindVertexArray(0);
    }

    // ========== Renderer_OpenGL Implementation ==========

    Renderer_OpenGL::Renderer_OpenGL() {
        InitializeShaders();
        SetupLineRendering();
    }

    Renderer_OpenGL::~Renderer_OpenGL() {
        CleanupShaders();
        if (m_LineVAO) glDeleteVertexArrays(1, &m_LineVAO);
        if (m_LineVBO) glDeleteBuffers(1, &m_LineVBO);
    }

    void Renderer_OpenGL::InitializeShaders() {
        // Compile vertex shader
        m_VertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(m_VertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(m_VertexShader);

        // Check vertex shader compilation
        GLint success;
        GLchar infoLog[512];
        glGetShaderiv(m_VertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(m_VertexShader, 512, nullptr, infoLog);
            // Vertex shader compilation failed
        }

        // Compile fragment shader
        m_FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(m_FragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(m_FragmentShader);

        glGetShaderiv(m_FragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(m_FragmentShader, 512, nullptr, infoLog);
            // Fragment shader compilation failed
        }

        // Link shader program
        m_ShaderProgram = glCreateProgram();
        glAttachShader(m_ShaderProgram, m_VertexShader);
        glAttachShader(m_ShaderProgram, m_FragmentShader);
        glLinkProgram(m_ShaderProgram);

        glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(m_ShaderProgram, 512, nullptr, infoLog);
            // Shader program linking failed
        }

        // Get uniform locations
        m_UniformProjection = glGetUniformLocation(m_ShaderProgram, "uProjection");
        m_UniformColor = glGetUniformLocation(m_ShaderProgram, "uColor");
    }

    void Renderer_OpenGL::CleanupShaders() {
        if (m_ShaderProgram) {
            glDeleteProgram(m_ShaderProgram);
            m_ShaderProgram = 0;
        }
        if (m_VertexShader) {
            glDeleteShader(m_VertexShader);
            m_VertexShader = 0;
        }
        if (m_FragmentShader) {
            glDeleteShader(m_FragmentShader);
            m_FragmentShader = 0;
        }
    }

    void Renderer_OpenGL::SetupLineRendering() {
        glGenVertexArrays(1, &m_LineVAO);
        glGenBuffers(1, &m_LineVBO);

        glBindVertexArray(m_LineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_LineVBO);

        // Position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        
        // TexCoord attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
        
        // Color attribute
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

        glBindVertexArray(0);
    }

    void Renderer_OpenGL::BeginFrame(const RenderTargetSpecs& target) {
        m_CurrentTarget = target;

        // Since ImGui rendering is disabled in Application.cpp, we need to:
        // 1. Set the viewport
        // 2. Clear the screen on the first frame or when needed
        
        glViewport(0, 0, target.width, target.height);
        
        // Clear the screen (since ImGui isn't doing it)
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Disable depth testing for 2D rendering
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        
        // Enable blending for transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Use our shader program
        glUseProgram(m_ShaderProgram);

        // Set up orthographic projection matrix
        glm::mat4 projection = glm::ortho(
            0.0f, static_cast<float>(target.width),
            static_cast<float>(target.height), 0.0f,
            -1.0f, 1.0f
        );
        glUniformMatrix4fv(m_UniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
    }

    void Renderer_OpenGL::Submit(const DrawCommandBuffer& cmdBuf) {
        for (const auto& cmd : cmdBuf.Data()) {
            switch (cmd.type) {
                case CommandType::PolygonBegin: {
                    const auto& polyBegin = cmd.polyBegin;
                    
                    // Start new polygon, store color, thickness, and filled state
                    m_PolygonVertices.clear();
                    m_PolygonColor = polyBegin.color;
                    m_PolygonThickness = polyBegin.thickness;
                    m_PolygonFilled = polyBegin.filled;
                    
                    // Add first vertex
                    Vertex v;
                    v.position = polyBegin.p;
                    v.texCoord = {0.0f, 0.0f};
                    v.color = polyBegin.color;
                    m_PolygonVertices.push_back(v);
                    break;
                }
                
                case CommandType::PolygonPoint: {
                    const auto& polyPoint = cmd.polyPoint;
                    
                    // Add intermediate point to polygon
                    Vertex v;
                    v.position = polyPoint.p;
                    v.texCoord = {0.0f, 0.0f};
                    v.color = m_PolygonColor;
                    m_PolygonVertices.push_back(v);
                    break;
                }
                
                case CommandType::PolygonEnd: {
                    const auto& polyEnd = cmd.polyEnd;
                    
                    // Add final point
                    Vertex v;
                    v.position = polyEnd.p;
                    v.texCoord = {1.0f, 1.0f};
                    v.color = m_PolygonColor;
                    m_PolygonVertices.push_back(v);
                    
                    // Now render all the vertices
                    if (m_PolygonVertices.size() >= 2) {
                        glBindVertexArray(m_LineVAO);
                        glBindBuffer(GL_ARRAY_BUFFER, m_LineVBO);
                        glBufferData(GL_ARRAY_BUFFER, 
                                    m_PolygonVertices.size() * sizeof(Vertex), 
                                    m_PolygonVertices.data(), 
                                    GL_DYNAMIC_DRAW);

                        if (m_PolygonFilled) {
                            // Render as filled polygon (triangle fan)
                            glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(m_PolygonVertices.size()));
                        } else {
                            // Render as line strip
                            glLineWidth(m_PolygonThickness);
                            glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(m_PolygonVertices.size()));
                        }
                    }
                    
                    // Clear polygon state
                    m_PolygonVertices.clear();
                    break;
                }
                
                case CommandType::Circle:
                case CommandType::QuadraticBezier:
                case CommandType::CubicBezier:
                    // Skip rendering circles and bezier curves as requested
                    break;
                    
                default:
                    break;
            }
        }

        glBindVertexArray(0);
    }

    void Renderer_OpenGL::EndFrame() {
        glUseProgram(0);
    }

    std::shared_ptr<IMesh> Renderer_OpenGL::CreateMesh() {
        return std::make_shared<OpenGLMesh>();
    }

    void Renderer_OpenGL::DrawMesh(const std::shared_ptr<IMesh>& mesh) {
        DrawMesh(mesh, ColorRGBA{1.0f, 1.0f, 1.0f, 1.0f});
    }

    void Renderer_OpenGL::DrawMesh(const std::shared_ptr<IMesh>& mesh, const ColorRGBA& color) {
        if (!mesh) return;

        mesh->Bind();
        
        const auto& indices = mesh->GetIndices();
        if (!indices.empty()) {
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        } else {
            const auto& vertices = mesh->GetVertices();
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
        }
        
        mesh->Unbind();
    }

}
