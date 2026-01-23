#pragma once

#include <vector>
#include <cstdint>
#include "DrawCommand.h"

namespace Core {

    struct Vertex {
        Vec3 position;
        Vec2 texCoord;
        ColorRGBA color;
    };

    class IMesh {
    public:
        virtual ~IMesh() = default;

        virtual void SetVertices(const std::vector<Vertex>& vertices) = 0;
        virtual void SetIndices(const std::vector<uint32_t>& indices) = 0;
        
        virtual const std::vector<Vertex>& GetVertices() const = 0;
        virtual const std::vector<uint32_t>& GetIndices() const = 0;
        
        virtual void Upload() = 0;
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;
    };

}
