
#include "Renderer_ImGui.h"
#include "../../vendor/include/imgui/imgui.h"

static ImDrawList* draw_list = nullptr;

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
		case CommandType::Line:
		{
			const auto& line = cmd.line;
			draw_list->AddLine(
				ImVec2(line.p0.x, line.p0.y),
				ImVec2(line.p1.x, line.p1.y),
				IM_COL32(
					static_cast<uint8_t>(line.color.r * 255),
					static_cast<uint8_t>(line.color.g * 255),
					static_cast<uint8_t>(line.color.b * 255),
					static_cast<uint8_t>(line.color.a * 255)
				),
				line.thickness
			);
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
		case CommandType::QuadraticBezier:
			// Implement quadratic bezier rendering if needed
			break;
		case CommandType::CubicBezier:
			// Implement cubic bezier rendering if needed
			break;
		default:
			break;
		}
	}

}

void Renderer_ImGui::EndFrame()
{

}