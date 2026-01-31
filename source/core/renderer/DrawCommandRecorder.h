#pragma once

#include "DrawCommandBuffer.h"
#include <glm/glm.hpp>

namespace Core {

	class DrawCommandRecorder {
	private:
		DrawCommandBuffer _cmdBuffer;
	public:
		DrawCommandRecorder() = default;
		
		inline void Clear() {
			_cmdBuffer.Clear();
		}
		
		// Polygon commands (3D)
		inline void PolygonBegin(const Vec3& p, float thickness, const ColorRGBA& color, bool filled = false) {
			_cmdBuffer.Push(DrawCommand::Add(PolygonBeginCmd{ p, thickness, color, filled }));
		}
		
		inline void PolygonPoint(const Vec3& p) {
			_cmdBuffer.Push(DrawCommand::Add(PolygonPointCmd{ p }));
		}
		
		inline void PolygonEnd(const Vec3& p) {
			_cmdBuffer.Push(DrawCommand::Add(PolygonEndCmd{ p }));
		}
		
		// Convenience method for drawing a simple line (2 points in 3D)
		inline void Line(const Vec3& p0, const Vec3& p1, float thickness, const ColorRGBA& color) {
			_cmdBuffer.Push(DrawCommand::Add(PolygonBeginCmd{ p0, thickness, color, false }));
			_cmdBuffer.Push(DrawCommand::Add(PolygonEndCmd{ p1 }));
		}
		
		// 2D overloads (convert to 3D with z=0)
		inline void PolygonBegin(const Vec2& p, float thickness, const ColorRGBA& color, bool filled = false) {
			_cmdBuffer.Push(DrawCommand::Add(PolygonBeginCmd{ {p.x, p.y, 0.0f}, thickness, color, filled }));
		}
		
		inline void PolygonPoint(const Vec2& p) {
			_cmdBuffer.Push(DrawCommand::Add(PolygonPointCmd{ {p.x, p.y, 0.0f} }));
		}
		
		inline void PolygonEnd(const Vec2& p) {
			_cmdBuffer.Push(DrawCommand::Add(PolygonEndCmd{ {p.x, p.y, 0.0f} }));
		}
		
		// Convenience method for drawing a simple line (2 points)
		inline void Line(const Vec2& p0, const Vec2& p1, float thickness, const ColorRGBA& color) {
			_cmdBuffer.Push(DrawCommand::Add(PolygonBeginCmd{ {p0.x, p0.y, 0.0f}, thickness, color, false }));
			_cmdBuffer.Push(DrawCommand::Add(PolygonEndCmd{ {p1.x, p1.y, 0.0f} }));
		}
		
		inline void Circle(const Vec2& p0, const float size, bool filled, const ColorRGBA& color, int num_segments = 12, float thickness = 1.0f)
		{
			_cmdBuffer.Push(DrawCommand::Add(CircleCmd{ p0, size, filled, color, num_segments, thickness }));
		 }
		
		inline void Cube(const glm::mat4& transform, const ColorRGBA& color) {
			_cmdBuffer.Push(DrawCommand::Add(CubeCmd{ transform, color }));
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