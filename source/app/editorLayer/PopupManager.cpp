#include "pch.h"
#include "PopupManager.h"

std::shared_ptr<PopupWindow> PopupManager::ShowPopup(const std::string& title, const std::string& message)
{
    auto popup = std::make_shared<PopupWindow>(title, message);
    popup->Open();
    _popups.push_back(popup);
    return popup;
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
