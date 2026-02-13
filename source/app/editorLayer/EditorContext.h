#pragma once
#include <set>
#include <memory>
#include <entt/entt.hpp>
#include "app/sceneLayer/types/Types.h"
#include "app/sceneLayer/Entity.h"

// Forward declarations
class SceneManager;
class PopupManager;
namespace Core {
    class EventBus;
    template<typename ValueTypes, typename ComponentTypes>
    class Applicator;
    template<typename ValueTypes>
    class UndoManager;
    enum class ClipboardType;
}

/// <summary>
/// EditorContext holds shared editor state and provides common operations
/// that can be accessed by multiple panels and UI components.
/// </summary>
class EditorContext
{
public:
    EditorContext(
        SceneManager& sceneManager,
        Core::EventBus& eventBus,
        Core::Applicator<AppFieldTypes, AppComponentTypes>& applicator,
        Core::UndoManager<AppFieldTypes>& undoManager,
        void* windowHandle
    );

    ~EditorContext() = default;

    // === Selection Management ===
    void RefreshSelectionState();
    const std::set<Entity>& GetSelectedEntities() const { return _selectedEntities; }
    void SetSelection(const std::set<Entity>& entities) { 
        _selectedEntities = entities; 
        _selectedEntityUUIDs.clear();
        for (const auto& entity : entities)
            _selectedEntityUUIDs.emplace(entity.UUID());
    }
    void AddToSelection(Entity entity) {
        _selectedEntities.insert(entity); _selectedEntityUUIDs.insert(entity.UUID());
    }
    void RemoveFromSelection(Entity entity) { _selectedEntities.erase(entity); _selectedEntityUUIDs.erase(entity.UUID()); }
    void ClearSelection() { _selectedEntities.clear(); _selectedEntityUUIDs.clear(); }
    bool IsSelected(Entity entity) const { return _selectedEntities.find(entity) != _selectedEntities.end(); }

    // === Edit Operations ===
    void Copy();
    void Cut();
    void Paste();
    void DeleteSelection();
    void Undo();
    void Redo();


    // === Scene Operations ===
    void NewScene(const std::string& name, bool makeActive = true);
    void SaveScene(const int sceneIndex);
    void SaveSceneAs(const int sceneIndex);
    void OpenScene();
    void RevertScene();
	void CloseScene(const int sceneIndex);
	bool IsSceneDirty(const int sceneIndex) const;

    // === Import Operations ===
    /// <summary>
    /// Import a 3D model (FBX, OBJ, etc.) into the active scene
    /// </summary>
    void ImportModel();

    // === Context Update ===
    /// <summary>
    /// Called when the active scene changes to update internal state
    /// </summary>
    void OnActiveSceneChanged();

    // === Applicator Wrapper Functions ===
    /// <summary>
    /// Begin recording operations for bundling into a single undo step
    /// </summary>
    void BeginUndo();
    
    /// <summary>
    /// End recording and push all bundled operations as a single undo step
    /// </summary>
    void EndUndo();
    
    /// <summary>
    /// Check if currently recording a bundle
    /// </summary>
    [[nodiscard]] bool IsRecording() const noexcept;

    // === Accessors ===
    SceneManager& sceneManager() { return _sceneManager; }
    Core::Applicator<AppFieldTypes, AppComponentTypes>& applicator();
    
    /// <summary>
    /// Set the popup manager reference (called by EditorApplicationLayer)
    /// </summary>
    void SetPopupManager(PopupManager* popupManager) { _popupManager = popupManager; }
    
    /// <summary>
    /// Get the popup manager for showing dialogs and popups
    /// </summary>
    PopupManager* GetPopupManager() { return _popupManager; }

	void SetImGuizmoMode(ImGuizmo::MODE mode) { _imGuizmoMode = mode; }
	ImGuizmo::MODE GetImGuizmoMode() const { return _imGuizmoMode; }
	void SetImGuizmoOperation(ImGuizmo::OPERATION operation) { _imGuizmoOperation = operation; }
	ImGuizmo::OPERATION GetImGuizmoOperation() const { return _imGuizmoOperation; }

private:
    // State
    std::set<Entity> _selectedEntities;
    std::set<Core::UUID> _selectedEntityUUIDs;
	ImGuizmo::MODE _imGuizmoMode = ImGuizmo::LOCAL;
	ImGuizmo::OPERATION _imGuizmoOperation = ImGuizmo::TRANSLATE;

    // Services (references to externally owned objects)
    SceneManager& _sceneManager;
    Core::EventBus& _eventBus;
    Core::Applicator<AppFieldTypes, AppComponentTypes>& _applicator;
    Core::UndoManager<AppFieldTypes>& _undoManager;
    void* _windowHandle;
    PopupManager* _popupManager = nullptr;
};