#pragma once
#include "PopupWindow.h"
#include <memory>
#include <vector>
#include <string>
#include <functional>

// Forward declaration
namespace Core {
    class Window;
}

/// <summary>
/// PopupManager manages multiple popup windows and ensures they are rendered properly.
/// This allows creating and managing popups from different parts of the editor.
/// </summary>
class PopupManager
{
public:
    PopupManager() = default;
    ~PopupManager() = default;

    /// <summary>
    /// Create and show a new popup window
    /// </summary>
    /// <param name="title">The title of the popup</param>
    /// <param name="message">The message to display</param>
    /// <returns>Shared pointer to the created popup for further configuration</returns>
    std::shared_ptr<PopupWindow> ShowPopup(const std::string& title, const std::string& message);

    /// <summary>
    /// Create a simple confirmation popup with OK and Cancel buttons
    /// </summary>
    /// <param name="title">The title of the popup</param>
    /// <param name="message">The message to display</param>
    /// <param name="onConfirm">Callback to execute when OK is clicked</param>
    /// <param name="onCancel">Optional callback to execute when Cancel is clicked</param>
    void ShowConfirmation(const std::string& title, const std::string& message,
        std::function<void()> onConfirm, std::function<void()> onCancel = nullptr);

    /// <summary>
    /// Show a blocking confirmation dialog that waits for user response
    /// </summary>
    /// <param name="title">The title of the popup</param>
    /// <param name="message">The message to display</param>
    /// <returns>True if user clicked OK/Yes, false if Cancel/No</returns>
    bool ShowBlockingConfirmation(const std::string& title, const std::string& message);

    /// <summary>
    /// Create a simple information popup with just an OK button
    /// </summary>
    /// <param name="title">The title of the popup</param>
    /// <param name="message">The message to display</param>
    /// <param name="onClose">Optional callback to execute when OK is clicked</param>
    void ShowInfo(const std::string& title, const std::string& message, 
        std::function<void()> onClose = nullptr);

    /// <summary>
    /// Create a warning popup with just an OK button
    /// </summary>
    /// <param name="title">The title of the popup</param>
    /// <param name="message">The warning message to display</param>
    /// <param name="onClose">Optional callback to execute when OK is clicked</param>
    void ShowWarning(const std::string& title, const std::string& message,
        std::function<void()> onClose = nullptr);

    /// <summary>
    /// Render all active popups. Must be called every frame.
    /// </summary>
    void Render();

    /// <summary>
    /// Close all open popups
    /// </summary>
    void CloseAll();

    /// <summary>
    /// Set the window reference for blocking popups
    /// </summary>
    void SetWindow(Core::Window* window) { _window = window; }

private:
    std::vector<std::shared_ptr<PopupWindow>> _popups;
    Core::Window* _window = nullptr;

    /// <summary>
    /// Remove closed popups from the list
    /// </summary>
    void CleanupClosedPopups();

    /// <summary>
    /// Run a mini render loop for blocking popups
    /// </summary>
    void RunBlockingPopupLoop(PopupWindow& popup);
};
