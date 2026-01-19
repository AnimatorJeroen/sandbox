#include "pch.h"
#include "PopupManager.h"
#include <core/Window.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

std::shared_ptr<PopupWindow> PopupManager::ShowPopup(const std::string& title, const std::string& message)
{
    auto popup = std::make_shared<PopupWindow>(title, message);
    popup->Open();
    _popups.push_back(popup);
    return popup;
}

void PopupManager::ShowPopupBlocking(std::shared_ptr<PopupWindow> popup)
{
    if (!_window || !popup)
        return;

    popup->Open();
    
    // Run blocking loop until popup is closed
    RunBlockingPopupLoop(*popup);
}

void PopupManager::ShowConfirmation(const std::string& title, const std::string& message,
    std::function<void()> onConfirm, std::function<void()> onCancel)
{
    auto popup = std::make_shared<PopupWindow>(title, message);
    
    // Add OK button
    popup->AddButton("OK", [onConfirm]() {
        if (onConfirm)
            onConfirm();
    }, true);
    
    // Add Cancel button
    popup->AddButton("Cancel", [onCancel]() {
        if (onCancel)
            onCancel();
    }, true);
    
    popup->Open();
    _popups.push_back(popup);
}

PopupResult PopupManager::ShowBlockingConfirmation(const std::string& title, const std::string& message)
{
    if (!_window)
        return PopupResult::None;

    auto popup = std::make_shared<PopupWindow>(title, message);
    PopupResult result = PopupResult::Cancel;

    // Add Yes button with result value 1
    popup->AddButton("Yes", [&result]() {
        result = PopupResult::Yes;
    }, true);

    // Add No button with result value 0
    popup->AddButton("No", [&result]() {
        result = PopupResult::No;
    }, true);

    // Add Cancel button with result value 0
    popup->AddButton("Cancel", [&result]() {
        result = PopupResult::Cancel;
    }, true);

    popup->Open();
    
    // Run blocking loop until popup is closed
    RunBlockingPopupLoop(*popup);

    return result;
}

void PopupManager::ShowInfo(const std::string& title, const std::string& message, 
    std::function<void()> onClose)
{
    auto popup = std::make_shared<PopupWindow>(title, message);
    
    popup->AddButton("OK", [onClose]() {
        if (onClose)
            onClose();
    }, true);
    
    popup->Open();
    _popups.push_back(popup);
}

void PopupManager::ShowWarning(const std::string& title, const std::string& message,
    std::function<void()> onClose)
{
    // Use the same implementation as ShowInfo but with a warning-style title
    ShowInfo(title, message, onClose);
}

void PopupManager::Render()
{
    // Render all active popups
    for (auto& popup : _popups)
    {
        if (popup && popup->IsOpen())
        {
            popup->Render();
        }
    }

    // Clean up closed popups
    CleanupClosedPopups();
}

void PopupManager::CloseAll()
{
    for (auto& popup : _popups)
    {
        if (popup)
            popup->Close();
    }
    _popups.clear();
}

void PopupManager::CleanupClosedPopups()
{
    // Remove popups that are no longer open
    _popups.erase(
        std::remove_if(_popups.begin(), _popups.end(),
            [](const std::shared_ptr<PopupWindow>& popup) {
                return !popup || !popup->IsOpen();
            }),
        _popups.end()
    );
}

void PopupManager::RunBlockingPopupLoop(PopupWindow& popup)
{
    if (!_window)
        return;

    GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(_window->GetHandle());
    
    // Run a mini render loop until the popup is closed
    while (popup.IsOpen()/* && !glfwWindowShouldClose(glfwWindow)*/)
    {
        // Poll events
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render the popup
        popup.Render();

        // Render ImGui
        ImGui::Render();
        
        // Clear and render
        int display_w, display_h;
        glfwGetFramebufferSize(glfwWindow, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        glfwSwapBuffers(glfwWindow);
    }
}
