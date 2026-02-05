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
#include "core/importer/MeshImporter.h"
#include "PopupManager.h"

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

// Helper function to recursively collect all descendants of an entity
static void CollectDescendantsRecursive(Entity entity, entt::registry* registry, std::set<entt::entity>& descendants)
{
    if (!entity || !entity.HasComponent<Children>())
        return;
    
    const Children& childrenComp = entity.GetComponent<Children>();
    for (entt::entity childHandle : childrenComp.children) {
        // Add the child
        descendants.insert(childHandle);
        
        // Recursively add descendants of this child
        Entity childEntity(childHandle, registry);
        CollectDescendantsRecursive(childEntity, registry, descendants);
    }
}

// Helper function to collect entities and all their descendants
static std::set<entt::entity> CollectEntitiesWithDescendants(const std::set<Entity>& entities, entt::registry* registry)
{
    std::set<entt::entity> result;
    
    for (const auto& entity : entities) {
        // Add the entity itself
        result.insert(entity.GetHandle());
        
        // Add all descendants recursively
        CollectDescendantsRecursive(entity, registry, result);
    }
    
    return result;
}

// === Edit Operations ===

void EditorContext::Copy()
{
    if (_selectedEntities.empty())
        return;

    auto scene = _sceneManager.GetActiveScene();
    if (!scene)
        return;

    // Collect selected entities and all their descendants
    std::set<entt::entity> entityHandles = CollectEntitiesWithDescendants(_selectedEntities, &scene->GetRegistry());

    _applicator.CopyToClipboard(entityHandles);
    LOG_DEBUG() << "Copied " << entityHandles.size() << " entities (including descendants) to clipboard";
}

void EditorContext::Cut()
{
    if (_selectedEntities.empty())
        return;

    auto scene = _sceneManager.GetActiveScene();
    if (!scene)
        return;

    // Collect selected entities and all their descendants
    std::set<entt::entity> entityHandles = CollectEntitiesWithDescendants(_selectedEntities, &scene->GetRegistry());

    // Copy to clipboard first
    _applicator.CopyToClipboard(entityHandles);

    // Then delete
    _applicator.BeginUndo();
    _applicator.CaptureDelete(entityHandles);
    _applicator.EndUndo();

    LOG_DEBUG() << "Cut " << entityHandles.size() << " entities (including descendants)";

    if (_sceneManager.GetActiveScene())
        _sceneManager.GetActiveScene()->RebuildChildrenForAllEntities();
}   

void EditorContext::Paste()
{
    _applicator.BeginUndo();
    _applicator.PasteFromClipboard(Core::ClipboardType::Entities);
    _applicator.EndUndo();

    LOG_DEBUG() << "Pasted entities from clipboard";

    if (_sceneManager.GetActiveScene())
        _sceneManager.GetActiveScene()->RebuildChildrenForAllEntities();
}

void EditorContext::DeleteSelection()
{
    if (_selectedEntities.empty())
        return;

    auto scene = _sceneManager.GetActiveScene();
    if (!scene)
        return;

    // Collect selected entities and all their descendants
    std::set<entt::entity> entityHandles = CollectEntitiesWithDescendants(_selectedEntities, &scene->GetRegistry());

    _applicator.BeginUndo();
    _applicator.CaptureDelete(entityHandles);
    _applicator.EndUndo();

    LOG_DEBUG() << "Deleted " << entityHandles.size() << " entities (including descendants)";

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

void EditorContext::NewScene(const std::string& name, bool makeActive)
{
    _sceneManager.CreateNewScene(name, makeActive);
}

void EditorContext::SaveScene(const int sceneIndex)
{
    auto scene = _sceneManager.GetScene(sceneIndex);
    if (scene && !scene->GetFilepath().empty())
    {
        // Save to current filepath
        LOG_DEBUG() << "Saving scene to: " << scene->GetFilepath();
        bool success = _sceneManager.SaveActiveScene(scene->GetFilepath());
        if (success)
            _undoManager.MarkContextDirty(scene->GetRegistry(), false);
    }
    else
    {
        // No filepath, show Save As dialog
        SaveSceneAs(sceneIndex);
    }
}

void EditorContext::SaveSceneAs(const int sceneIndex)
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
        LOG_DEBUG() << "Saving scene as: " << result.value();
        bool success = _sceneManager.SaveScene(sceneIndex, result.value());
        if (success)
            _undoManager.MarkContextDirty(scene->GetRegistry(), false);
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

void EditorContext::CloseScene(const int sceneIndex)
{
	auto scene = _sceneManager.GetScene(sceneIndex);
    if (scene == nullptr)
		return;
    _sceneManager.CloseScene(sceneIndex);
}

bool EditorContext::IsSceneDirty(const int sceneIndex) const
{
    auto scene = _sceneManager.GetScene(sceneIndex);
    return scene ? _undoManager.IsContextDirty(
        scene->GetRegistry()) : false;
}

// === Import Operations ===

void EditorContext::ImportModel()
{
    auto scene = _sceneManager.GetActiveScene();
    if (!scene)
    {
        LOG_WARN() << "Cannot import model: No active scene";
        if (_popupManager)
        {
            _popupManager->ShowWarning("Import Failed", "No active scene. Please create or open a scene first.");
        }
        return;
    }

    // Show file browser for model selection
    Core::BrowserWindow browserWindow(_windowHandle);
    auto result = browserWindow.OpenFile(
        "Import 3D Model",
        {
            Core::FileFilter("FBX Files", "*.fbx"),
            Core::FileFilter("OBJ Files", "*.obj"),
            Core::FileFilter("GLTF Files", "*.gltf;*.glb"),
            Core::FileFilter("Collada Files", "*.dae"),
            Core::FileFilter("All Supported", "*.fbx;*.obj;*.gltf;*.glb;*.dae;*.blend;*.3ds"),
            Core::FileFilter("All Files", "*.*")
        });

    if (!result.has_value())
    {
        LOG_DEBUG() << "Model import cancelled by user";
        return;
    }

    LOG_INFO() << "Importing model from: " << result.value();

    // Import the model using MeshImporter
    Core::MeshImporter importer;
    
    // Get selected entity as parent (if any)
    Entity parent;
    if (!_selectedEntities.empty())
    {
        parent = *_selectedEntities.begin();
    }

    _applicator.BeginUndo();
    bool success = importer.ImportModel(result.value(), scene.get(), parent ? &parent : nullptr);
    _applicator.EndUndo();

    if (success)
    {
        LOG_INFO() << "Model imported successfully";
        if (_popupManager)
        {
            _popupManager->ShowInfo("Import Successful", "3D model imported successfully!");
        }
        
        // Rebuild hierarchy after import
        scene->RebuildChildrenForAllEntities();
    }
    else
    {
        LOG_ERROR() << "Model import failed: " << importer.GetLastError();
        if (_popupManager)
        {
            _popupManager->ShowWarning("Import Failed",
                "Failed to import model:\n" + importer.GetLastError());
        }
    }
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
