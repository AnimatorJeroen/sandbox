#include "pch.h"
#include "EditorContext.h"
#include "app/sceneLayer/SceneManager.h"
#include "app/sceneLayer/Scene.h"
#include <core/event/EventBus.h>
#include <core/applicator/Applicator.h>
#include <core/applicator/UndoManager.h>
#include <core/applicator/Clipboard.h>
#include <core/BrowserWindow.h>
#include <core/Logger.h>
#include "app/event/SceneEvent.h"

EditorContext::EditorContext(
    SceneManager& sceneManager,
    Core::EventBus& eventBus,
    Core::Applicator<AppFieldTypes, AppComponentTypes>& applicator,
    Core::UndoManager<AppFieldTypes>& undoManager,
    void* windowHandle
)
    : _sceneManager(sceneManager)
    , _eventBus(eventBus)
    , _applicator(applicator)
    , _undoManager(undoManager)
    , _windowHandle(windowHandle)
{
}

Core::Applicator<AppFieldTypes, AppComponentTypes>& EditorContext::applicator()
{
    return _applicator;
}

// === Edit Operations ===

void EditorContext::Copy()
{
    if (_selectedEntities.empty())
        return;

    _applicator.CopyToClipboard(_selectedEntities);
    LOG_DEBUG() << "Copied " << _selectedEntities.size() << " entities to clipboard";
}

void EditorContext::Cut()
{
    if (_selectedEntities.empty())
        return;

    // Copy to clipboard first
    _applicator.CopyToClipboard(_selectedEntities);

    // Then delete
    _applicator.BeginUndo();
    _applicator.CaptureDelete(_selectedEntities);
    _applicator.EndUndo();

    LOG_DEBUG() << "Cut " << _selectedEntities.size() << " entities";
}

void EditorContext::Paste()
{
    _applicator.BeginUndo();
    _applicator.PasteFromClipboard(Core::ClipboardType::Entities);
    _applicator.EndUndo();

    LOG_DEBUG() << "Pasted entities from clipboard";
}

void EditorContext::DeleteSelection()
{
    if (_selectedEntities.empty())
        return;

    _applicator.BeginUndo();
    _applicator.CaptureDelete(_selectedEntities);
    _applicator.EndUndo();

    LOG_DEBUG() << "Deleted " << _selectedEntities.size() << " entities";

    // Clear selection after deletion
    _selectedEntities.clear();
}

void EditorContext::Undo()
{
    bool handled = _undoManager.Undo();
    LOG_DEBUG() << "Undo handled: " << handled;
}

void EditorContext::Redo()
{
    bool handled = _undoManager.Redo();
    LOG_DEBUG() << "Redo handled: " << handled;
}

// === Scene Operations ===

void EditorContext::SaveScene(const size_t sceneIndex)
{
    auto scene = _sceneManager.GetScene(sceneIndex);
    if (scene && !scene->GetFilepath().empty())
    {
        // Save to current filepath
        LOG_DEBUG() << "Saving scene to: " << scene->GetFilepath();
        _sceneManager.SaveActiveScene(scene->GetFilepath());
    }
    else
    {
        // No filepath, show Save As dialog
        SaveSceneAs(sceneIndex);
    }
}

void EditorContext::SaveSceneAs(const size_t sceneIndex)
{
    auto scene = _sceneManager.GetScene(sceneIndex);
    if (scene == nullptr)
        return;

    Core::BrowserWindow browserWindow(_windowHandle);
    auto result = browserWindow.SaveFile(
        "Save Scene As",
        {
            Core::FileFilter("Scene Files", "*.scene"),
            Core::FileFilter("Binary Files", "*.dat"),
            Core::FileFilter("All Files", "*.*")
        },
        "",
        "scene");

    if (result.has_value())
    {
        _sceneManager.SaveScene(sceneIndex, result.value());
        LOG_DEBUG() << "Saving scene as: " << result.value();
    }
}

void EditorContext::OpenScene()
{
    Core::BrowserWindow browserWindow(_windowHandle);
    auto result = browserWindow.OpenFile(
        "Open Scene",
        {
            Core::FileFilter("Scene Files", "*.scene"),
            Core::FileFilter("Binary Files", "*.dat"),
            Core::FileFilter("All Files", "*.*")
        });

    if (result.has_value())
    {
        _eventBus.PushEvent<RequestLoadSceneEvent>(
            RequestLoadSceneEvent(result.value()));
        LOG_DEBUG() << "Loading scene: " << result.value();
    }
}

void EditorContext::RevertScene()
{
    auto scene = _sceneManager.GetActiveScene();
    if (scene == nullptr)
        return;

    auto filePath = scene->GetFilepath();
    if (filePath.empty())
        return;

    _eventBus.PushImmediateEvent<RequestCloseSceneEvent>(
        RequestCloseSceneEvent(_sceneManager.GetActiveSceneIndex()));

    _eventBus.PushImmediateEvent<RequestLoadSceneEvent>(
        RequestLoadSceneEvent(filePath));

    LOG_DEBUG() << "Reverting scene: " << filePath;
}

void EditorContext::CloseScene(const size_t sceneIndex)
{
	auto scene = _sceneManager.GetScene(sceneIndex);
    if (scene == nullptr)
		return;
    _sceneManager.CloseScene(sceneIndex);
}

// === Context Update ===

void EditorContext::OnActiveSceneChanged()
{
    // Clear selection when scene changes
    _selectedEntities.clear();

    // Update undo manager context
    auto scene = _sceneManager.GetActiveScene();
    _undoManager.SetContext(scene ? &scene->GetRegistry() : nullptr);

    LOG_DEBUG() << "Editor context updated for new active scene";
}

// === Applicator Wrapper Functions ===

void EditorContext::BeginUndo()
{
    _applicator.BeginUndo();
}

void EditorContext::EndUndo()
{
    _applicator.EndUndo();
}

bool EditorContext::IsRecording() const noexcept
{
    return _applicator.IsRecording();
}
