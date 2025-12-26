#pragma once

#include "DrawCommandBuffer.h"

namespace Core {

	class DrawCommandRecorder {
	private:
		DrawCommandBuffer _cmdBuffer;
	public:
		DrawCommandRecorder() = default;
		inline void Clear() {
			_cmdBuffer.Clear();
		}
		inline void Line(const Vec2& p0, const Vec2& p1, float thickness, const ColorRGBA& color) {
			_cmdBuffer.Push(DrawCommand::Add(LineCmd{ p0, p1, thickness, color }));
		}
		inline void Circle(const Vec2& p0, const float size, bool filled, const ColorRGBA& color, int num_segments = 12, float thickness = 1.0f)
		{
			_cmdBuffer.Push(DrawCommand::Add(CircleCmd{ p0, size, filled, color, num_segments, thickness }));
		}
		inline void QuadraticBezier(const Vec2& p0, const Vec2& p1, const Vec2& p2, float thickness, const ColorRGBA& color) {
			_cmdBuffer.Push(DrawCommand::Add(QuadraticBezierCmd{ p0, p1, p2, thickness, color }));
		}
		inline void CubicBezier(const Vec2& p0, const Vec2& p1, const Vec2& p2, const Vec2& p3, float thickness, const ColorRGBA& color) {
			_cmdBuffer.Push(DrawCommand::Add(CubicBezierCmd{ p0, p1, p2, p3, thickness, color }));
		}
		inline const DrawCommandBuffer& GetCommandBuffer() const {
			return _cmdBuffer;
		}
	};
}