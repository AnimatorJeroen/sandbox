#include "pch.h"
#include "Renderer_ImGui.h"
#include "imgui/imgui.h"
#include <vector>

namespace Core {

	static ImDrawList* draw_list = nullptr;
	static std::vector<ImVec2> polygon_points;
	static ImU32 polygon_color = IM_COL32(255, 255, 255, 255);
	static float polygon_thickness = 1.0f;
	static bool polygon_filled = false;

	void Renderer_ImGui::BeginFrame(const IRenderer::RenderTargetSpecs& target)
	{
		draw_list = ImGui::GetBackgroundDrawList();
	}

	void Renderer_ImGui::Submit(const DrawCommandBuffer& cmdBuf)
	{
		for (const auto& cmd : cmdBuf.Data())
		{
			switch (cmd.type)
			{
			case CommandType::PolygonBegin:
			{
				const auto& polyBegin = cmd.polyBegin;
				polygon_points.clear();
				polygon_points.push_back(ImVec2(polyBegin.p.x, polyBegin.p.y));
				polygon_color = IM_COL32(
					static_cast<uint8_t>(polyBegin.color.r * 255),
					static_cast<uint8_t>(polyBegin.color.g * 255),
					static_cast<uint8_t>(polyBegin.color.b * 255),
					static_cast<uint8_t>(polyBegin.color.a * 255)
				);
				polygon_thickness = polyBegin.thickness;
				polygon_filled = polyBegin.filled;
				break;
			}
			
			case CommandType::PolygonPoint:
			{
				const auto& polyPoint = cmd.polyPoint;
				polygon_points.push_back(ImVec2(polyPoint.p.x, polyPoint.p.y));
				break;
			}
			
			case CommandType::PolygonEnd:
			{
				const auto& polyEnd = cmd.polyEnd;
				polygon_points.push_back(ImVec2(polyEnd.p.x, polyEnd.p.y));
				
				// Draw polygon
				if (polygon_points.size() >= 2) {
					if (polygon_filled) {
						// Draw filled convex polygon
						draw_list->AddConvexPolyFilled(
							polygon_points.data(),
							static_cast<int>(polygon_points.size()),
							polygon_color
						);
					} else {
						// Draw polyline (outline)
						draw_list->AddPolyline(
							polygon_points.data(),
							static_cast<int>(polygon_points.size()),
							polygon_color,
							false, // not closed
							polygon_thickness
						);
					}
				}
				polygon_points.clear();
				break;
			}
			
			case CommandType::Circle:
			{
				const auto& circle = cmd.circle;
				if (circle.filled) {
					draw_list->AddCircleFilled(
						ImVec2(circle.p.x, circle.p.y),
						circle.size,
						IM_COL32(
							static_cast<uint8_t>(circle.color.r * 255),
							static_cast<uint8_t>(circle.color.g * 255),
							static_cast<uint8_t>(circle.color.b * 255),
							static_cast<uint8_t>(circle.color.a * 255)
						),
						circle.num_segments
					);
				}
				else {
					draw_list->AddCircle(
						ImVec2(circle.p.x, circle.p.y),
						circle.size,
						IM_COL32(
							static_cast<uint8_t>(circle.color.r * 255),
							static_cast<uint8_t>(circle.color.g * 255),
							static_cast<uint8_t>(circle.color.b * 255),
							static_cast<uint8_t>(circle.color.a * 255)
						),
						circle.num_segments,
						circle.thickness
					);
				}
				break;
			}
			case CommandType::Cube:
				// ImGui doesn't support 3D rendering, skip cubes
				break;
			case CommandType::QuadraticBezier:
				// Implement quadratic bezier rendering if needed
				break;
			case CommandType::CubicBezier:
				draw_list->AddBezierCurve(
					ImVec2(cmd.cbez.p0.x, cmd.cbez.p0.y),
					ImVec2(cmd.cbez.p1.x, cmd.cbez.p1.y),
					ImVec2(cmd.cbez.p2.x, cmd.cbez.p2.y),
					ImVec2(cmd.cbez.p3.x, cmd.cbez.p3.y),
					IM_COL32(
						static_cast<uint8_t>(cmd.cbez.color.r * 255),
						static_cast<uint8_t>(cmd.cbez.color.g * 255),
						static_cast<uint8_t>(cmd.cbez.color.b * 255),
						static_cast<uint8_t>(cmd.cbez.color.a * 255)
					),
					cmd.cbez.thickness
				);
				break;
			default:
				break;
			}
		}
	}

	void Renderer_ImGui::EndFrame()
	{
	}
}