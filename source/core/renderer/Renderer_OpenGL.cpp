#include "pch.h"
#include "Renderer_OpenGL.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Core {

    // Vertex shader source (for 3D polygons)
    static const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        layout (location = 2) in vec4 aColor;
        
        uniform mat4 uViewProjection;
        
        out vec2 TexCoord;
        out vec4 Color;
        
        void main()
        {
            gl_Position = uViewProjection * vec4(aPos, 1.0);
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

    // 3D Cube vertex shader
    static const char* cubeVertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        
        uniform mat4 uMVP;
        uniform vec4 uColor;
        
        out vec4 Color;
        out vec3 Normal;
        
        void main()
        {
            gl_Position = uMVP * vec4(aPos, 1.0);
            Color = uColor;
            Normal = aNormal;
        }
    )";

    // 3D Cube fragment shader
    static const char* cubeFragmentShaderSource = R"(
        #version 330 core
        in vec4 Color;
        in vec3 Normal;
        
        out vec4 FragColor;
        
        void main()
        {
            // Simple lighting
            vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
            float diff = max(dot(normalize(Normal), lightDir), 0.3);
            FragColor = vec4(Color.rgb * diff, Color.a);
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
        // Position (3D now)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        
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
        // Initialize matrices to identity
        m_ViewMatrix = glm::mat4(1.0f);
        m_ProjectionMatrix = glm::mat4(1.0f);
        
        InitializeShaders();
        SetupLineRendering();
        SetupCubeRendering();
        CreateDefaultCube();
    }

    Renderer_OpenGL::~Renderer_OpenGL() {
        CleanupShaders();
        if (m_LineVAO) glDeleteVertexArrays(1, &m_LineVAO);
        if (m_LineVBO) glDeleteBuffers(1, &m_LineVBO);
        if (m_CubeVAO) glDeleteVertexArrays(1, &m_CubeVAO);
        if (m_CubeVBO) glDeleteBuffers(1, &m_CubeVBO);
        if (m_CubeEBO) glDeleteBuffers(1, &m_CubeEBO);
    }

    void Renderer_OpenGL::InitializeShaders() {
        // Compile 2D vertex shader
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

        // Compile 2D fragment shader
        m_FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(m_FragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(m_FragmentShader);

        glGetShaderiv(m_FragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(m_FragmentShader, 512, nullptr, infoLog);
            // Fragment shader compilation failed
        }

        // Link 2D shader program
        m_ShaderProgram = glCreateProgram();
        glAttachShader(m_ShaderProgram, m_VertexShader);
        glAttachShader(m_ShaderProgram, m_FragmentShader);
        glLinkProgram(m_ShaderProgram);

        glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(m_ShaderProgram, 512, nullptr, infoLog);
            // Shader program linking failed
        }

        // Get 2D uniform locations
        m_UniformProjection = glGetUniformLocation(m_ShaderProgram, "uProjection");
        m_UniformColor = glGetUniformLocation(m_ShaderProgram, "uColor");
    }

    void Renderer_OpenGL::CleanupShaders() {
        // Cleanup 2D shaders
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
        
        // Cleanup 3D cube shaders
        if (m_CubeShaderProgram) {
            glDeleteProgram(m_CubeShaderProgram);
            m_CubeShaderProgram = 0;
        }
        if (m_CubeVertexShader) {
            glDeleteShader(m_CubeVertexShader);
            m_CubeVertexShader = 0;
        }
        if (m_CubeFragmentShader) {
            glDeleteShader(m_CubeFragmentShader);
            m_CubeFragmentShader = 0;
        }
    }

    void Renderer_OpenGL::SetupLineRendering() {
        glGenVertexArrays(1, &m_LineVAO);
        glGenBuffers(1, &m_LineVBO);

        glBindVertexArray(m_LineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_LineVBO);

        // Position attribute (3D now)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        
        // TexCoord attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
        
        // Color attribute
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

        glBindVertexArray(0);
    }

    void Renderer_OpenGL::SetupCubeRendering() {
        // Compile 3D cube vertex shader
        m_CubeVertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(m_CubeVertexShader, 1, &cubeVertexShaderSource, nullptr);
        glCompileShader(m_CubeVertexShader);

        GLint success;
        GLchar infoLog[512];
        glGetShaderiv(m_CubeVertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(m_CubeVertexShader, 512, nullptr, infoLog);
        }

        // Compile 3D cube fragment shader
        m_CubeFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(m_CubeFragmentShader, 1, &cubeFragmentShaderSource, nullptr);
        glCompileShader(m_CubeFragmentShader);

        glGetShaderiv(m_CubeFragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(m_CubeFragmentShader, 512, nullptr, infoLog);
        }

        // Link 3D cube shader program
        m_CubeShaderProgram = glCreateProgram();
        glAttachShader(m_CubeShaderProgram, m_CubeVertexShader);
        glAttachShader(m_CubeShaderProgram, m_CubeFragmentShader);
        glLinkProgram(m_CubeShaderProgram);

        glGetProgramiv(m_CubeShaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(m_CubeShaderProgram, 512, nullptr, infoLog);
        }

        // Get 3D uniform locations
        m_CubeUniformMVP = glGetUniformLocation(m_CubeShaderProgram, "uMVP");
        m_CubeUniformColor = glGetUniformLocation(m_CubeShaderProgram, "uColor");
    }

    void Renderer_OpenGL::CreateDefaultCube() {
        // Cube vertices with positions and normals
        float cubeVertices[] = {
            // Positions          // Normals
            // Front face
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            // Back face
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            // Top face
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
            // Bottom face
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
            // Right face
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
            // Left face
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        };

        uint32_t cubeIndices[] = {
            0, 1, 2,  2, 3, 0,    // Front
            4, 6, 5,  6, 4, 7,    // Back
            8, 9, 10, 10, 11, 8,  // Top
            12, 14, 13, 14, 12, 15, // Bottom
            16, 17, 18, 18, 19, 16, // Right
            20, 22, 21, 22, 20, 23  // Left
        };

        m_CubeIndexCount = 36;

        glGenVertexArrays(1, &m_CubeVAO);
        glGenBuffers(1, &m_CubeVBO);
        glGenBuffers(1, &m_CubeEBO);

        glBindVertexArray(m_CubeVAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_CubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_CubeEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

        // Position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

        // Normal attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

        glBindVertexArray(0);
    }

    void Renderer_OpenGL::BeginFrame(const RenderTargetSpecs& target) {
        m_CurrentTarget = target;

        // Clear cube instances from previous frame
        m_CubeInstances.clear();

        // Note: View and projection matrices are now set externally via SetViewMatrix/SetProjectionMatrix

        // Since ImGui rendering is disabled in Application.cpp, we need to:
        // 1. Set the viewport
        // 2. Clear the screen on the first frame or when needed
        
        glViewport(0, 0, target.width, target.height);
        
        // Clear the screen (since ImGui isn't doing it)
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Enable depth testing for 3D rendering
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        
        // Enable blending for transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Use our shader program
        glUseProgram(m_ShaderProgram);

        // Set up view-projection matrix for 3D polygon rendering
        glm::mat4 viewProjection = m_ProjectionMatrix * m_ViewMatrix;
        GLint uniformViewProj = glGetUniformLocation(m_ShaderProgram, "uViewProjection");
        glUniformMatrix4fv(uniformViewProj, 1, GL_FALSE, glm::value_ptr(viewProjection));
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
                        // Only bind shader if it's not already bound
                        GLint currentProgram = 0;
                        glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
                        if (currentProgram != static_cast<GLint>(m_ShaderProgram)) {
                            glUseProgram(m_ShaderProgram);
                            glm::mat4 viewProjection = m_ProjectionMatrix * m_ViewMatrix;
                            GLint uniformViewProj = glGetUniformLocation(m_ShaderProgram, "uViewProjection");
                            glUniformMatrix4fv(uniformViewProj, 1, GL_FALSE, glm::value_ptr(viewProjection));
                        }

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
                
                case CommandType::Cube: {
                    const auto& cube = cmd.cube;
                    // Collect cube for instanced rendering in EndFrame
                    m_CubeInstances.push_back({cube.transform, cube.color});
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
        // Render all collected cubes
        if (!m_CubeInstances.empty()) {
            // Enable depth testing for 3D rendering
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            
            glUseProgram(m_CubeShaderProgram);
            glBindVertexArray(m_CubeVAO);
            
            // Use the view and projection matrices set in BeginFrame
            // Render each cube instance
            for (const auto& instance : m_CubeInstances) {
                glm::mat4 mvp = m_ProjectionMatrix * m_ViewMatrix * instance.transform;
                glUniformMatrix4fv(m_CubeUniformMVP, 1, GL_FALSE, glm::value_ptr(mvp));
                glUniform4f(m_CubeUniformColor, 
                           instance.color.r, instance.color.g, 
                           instance.color.b, instance.color.a);
                
                glDrawElements(GL_TRIANGLES, m_CubeIndexCount, GL_UNSIGNED_INT, 0);
            }
            
            glBindVertexArray(0);
            
            // Restore 2D rendering state
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        
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
    
    void Renderer_OpenGL::SetViewMatrix(const glm::mat4& view) {
        m_ViewMatrix = view;
    }
    
    void Renderer_OpenGL::SetProjectionMatrix(const glm::mat4& projection) {
        m_ProjectionMatrix = projection;
    }

}
