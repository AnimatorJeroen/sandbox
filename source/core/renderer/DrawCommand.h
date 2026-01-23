#pragma once
#include <cstdint>
#include <array>
#include <glm/glm.hpp>

namespace Core {

    enum class CommandType : uint8_t {
        PolygonBegin,
        PolygonPoint,
        PolygonEnd,
        Circle,
        Cube,
        QuadraticBezier,
        CubicBezier,
    };

    struct ColorRGBA {
        float r, g, b, a; // premultiplied optional
    };

    struct Vec2 {
        float x, y;
    };
    
    struct Vec3 {
        float x, y, z;
    };

    // Keep command payloads POD and compact.
    struct PolygonBeginCmd {
        Vec3 p;
        float thickness;
        ColorRGBA color;
        bool filled;
    };

    struct PolygonPointCmd {
        Vec3 p;
    };

    struct PolygonEndCmd {
        Vec3 p;
    };

    struct CircleCmd {
        Vec2 p;
        float size;
        bool filled;
        ColorRGBA color;
        int num_segments;
        float thickness;
    };

    struct CubeCmd {
        glm::mat4 transform;
        ColorRGBA color;
    };

    struct QuadraticBezierCmd {
        Vec2 p0, p1, p2;   // start, control, end
        float thickness;
        ColorRGBA color;
    };

    struct CubicBezierCmd {
        Vec2 p0, p1, p2, p3; // start, control1, control2, end
        float thickness;
        ColorRGBA color;
    };

    // Tagged union for commands
    struct DrawCommand {
        CommandType type;
        // padding may be added by the compiler—ok for simplicity
        union {
            PolygonBeginCmd polyBegin;
            PolygonPointCmd polyPoint;
            PolygonEndCmd polyEnd;
            CircleCmd circle;
            CubeCmd cube;
            QuadraticBezierCmd qbez;
            CubicBezierCmd cbez;
        };

        // Helpers to construct commands
        static DrawCommand Add(const PolygonBeginCmd& c) {
            DrawCommand cmd; cmd.type = CommandType::PolygonBegin; cmd.polyBegin = c; return cmd;
        }
        static DrawCommand Add(const PolygonPointCmd& c) {
            DrawCommand cmd; cmd.type = CommandType::PolygonPoint; cmd.polyPoint = c; return cmd;
        }
        static DrawCommand Add(const PolygonEndCmd& c) {
            DrawCommand cmd; cmd.type = CommandType::PolygonEnd; cmd.polyEnd = c; return cmd;
        }
        static DrawCommand Add(const CircleCmd& c) {
            DrawCommand cmd; cmd.type = CommandType::Circle; cmd.circle = c; return cmd;
        }
        static DrawCommand Add(const CubeCmd& c) {
            DrawCommand cmd; cmd.type = CommandType::Cube; cmd.cube = c; return cmd;
        }
        static DrawCommand Add(const QuadraticBezierCmd& c) {
            DrawCommand cmd; cmd.type = CommandType::QuadraticBezier; cmd.qbez = c; return cmd;
        }
        static DrawCommand Add(const CubicBezierCmd& c) {
            DrawCommand cmd; cmd.type = CommandType::CubicBezier; cmd.cbez = c; return cmd;
        }
    };
}
