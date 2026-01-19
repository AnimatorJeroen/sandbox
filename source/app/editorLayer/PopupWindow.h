#pragma once
#include <string>
#include <vector>
#include <functional>

/// <summary>
/// Represents a button in a popup window with text and callback
/// </summary>
struct PopupButton
{
    std::string text;
    std::function<void()> callback;
    bool closeOnClick = true; // Whether clicking this button should close the popup

    PopupButton(const std::string& buttonText, std::function<void()> buttonCallback, bool shouldClose = true)
        : text(buttonText), callback(std::move(buttonCallback)), closeOnClick(shouldClose)
    {
    }
};

/// <summary>
/// PopupWindow provides a modal dialog system using ImGui that blocks input
/// until the user interacts with it. Supports custom buttons and callbacks.
/// </summary>
class PopupWindow
{
public:
    PopupWindow(const std::string& title, const std::string& message);
    ~PopupWindow() = default;

    /// <summary>
    /// Add a button to the popup window
    /// </summary>
    /// <param name="text">The text displayed on the button</param>
    /// <param name="callback">The function to call when the button is clicked</param>
    /// <param name="closeOnClick">Whether clicking the button should close the popup (default: true)</param>
    void AddButton(const std::string& text, std::function<void()> callback, bool closeOnClick = true);

    /// <summary>
    /// Open the popup window
    /// </summary>
    void Open();

    /// <summary>
    /// Close the popup window
    /// </summary>
    void Close();

    /// <summary>
    /// Check if the popup is currently open
    /// </summary>
    bool IsOpen() const { return _isOpen; }

    /// <summary>
    /// Render the popup window. Must be called every frame.
    /// </summary>
    void Render();

    /// <summary>
    /// Set the message text of the popup
    /// </summary>
    void SetMessage(const std::string& message) { _message = message; }

    /// <summary>
    /// Set the title of the popup
    /// </summary>
    void SetTitle(const std::string& title) { _title = title; }

private:
    std::string _title;
    std::string _message;
    std::vector<PopupButton> _buttons;
    bool _isOpen = false;
    bool _needsOpen = false; // Flag to trigger ImGui::OpenPopup on next frame
};
