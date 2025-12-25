#pragma once
#include <cstdint>
#include <array>

enum class CommandType : uint8_t {
    Line,
    Circle,
    QuadraticBezier,
    CubicBezier,
};

struct ColorRGBA {
    float r, g, b, a; // premultiplied optional
};

struct Vec2 {
    float x, y;
};

// Keep command payloads POD and compact.
struct LineCmd {
    Vec2 p0;
    Vec2 p1;
    float thickness;
    ColorRGBA color;
};

struct CircleCmd {
    Vec2 p;
    float size;
	bool filled;
    ColorRGBA color;
	int num_segments;
	float thickness;
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
        LineCmd line;
		CircleCmd circle;
        QuadraticBezierCmd qbez;
        CubicBezierCmd cbez;
    };

    // Helpers to construct commands
    static DrawCommand Add(const LineCmd& c) {
        DrawCommand cmd; cmd.type = CommandType::Line; cmd.line = c; return cmd;
    }
	static DrawCommand Add(const CircleCmd& c) {
		DrawCommand cmd; cmd.type = CommandType::Circle; cmd.circle = c; return cmd;
	}
    static DrawCommand Add(const QuadraticBezierCmd& c) {
        DrawCommand cmd; cmd.type = CommandType::QuadraticBezier; cmd.qbez = c; return cmd;
    }
    static DrawCommand Add(const CubicBezierCmd& c) {
        DrawCommand cmd; cmd.type = CommandType::CubicBezier; cmd.cbez = c; return cmd;
    }
};
