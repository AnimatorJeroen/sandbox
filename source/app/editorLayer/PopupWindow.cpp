#include "pch.h"
#include "PopupWindow.h"
#include <imgui/imgui.h>

PopupWindow::PopupWindow(const std::string& title, const std::string& message)
    : _title(title), _message(message), _isOpen(false), _needsOpen(false)
{
}

void PopupWindow::AddButton(const std::string& text, std::function<void()> callback, bool closeOnClick)
{
    _buttons.emplace_back(text, std::move(callback), closeOnClick);
}

void PopupWindow::Open()
{
    _isOpen = true;
    _needsOpen = true;
}

void PopupWindow::Close()
{
    _isOpen = false;
    _needsOpen = false;
}

void PopupWindow::Render()
{
    if (!_isOpen)
        return;

    // Open the popup on the first frame after Open() is called
    if (_needsOpen)
    {
        ImGui::OpenPopup(_title.c_str());
        _needsOpen = false;
    }

    // Center the popup window
    ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    // BeginPopupModal blocks input to other windows
    if (ImGui::BeginPopupModal(_title.c_str(), &_isOpen, ImGuiWindowFlags_AlwaysAutoResize))
    {
        // Display message text
        ImGui::TextWrapped("%s", _message.c_str());
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Render buttons
        if (!_buttons.empty())
        {
            // Calculate total button width to center them
            float totalWidth = 0.0f;
            for (size_t i = 0; i < _buttons.size(); ++i)
            {
                totalWidth += ImGui::CalcTextSize(_buttons[i].text.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f + 20.0f;
                if (i < _buttons.size() - 1)
                    totalWidth += ImGui::GetStyle().ItemSpacing.x;
            }

            // Center the buttons
            float availWidth = ImGui::GetContentRegionAvail().x;
            if (totalWidth < availWidth)
            {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (availWidth - totalWidth) * 0.5f);
            }

            // Render each button
            for (size_t i = 0; i < _buttons.size(); ++i)
            {
                if (i > 0)
                    ImGui::SameLine();

                if (ImGui::Button(_buttons[i].text.c_str(), ImVec2(120, 0)))
                {
                    // Execute the callback
                    if (_buttons[i].callback)
                        _buttons[i].callback();

                    // Close popup if requested
                    if (_buttons[i].closeOnClick)
                    {
                        Close();
                        ImGui::CloseCurrentPopup();
                    }
                }
            }
        }

        ImGui::EndPopup();
    }

    // If the popup was closed by clicking outside or pressing Escape
    if (!ImGui::IsPopupOpen(_title.c_str()) && _isOpen && !_needsOpen)
    {
        _isOpen = false;
    }
}
