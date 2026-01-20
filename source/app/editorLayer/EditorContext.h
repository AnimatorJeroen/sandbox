#pragma once
#include <set>
#include <memory>
#include <entt/entt.hpp>
#include "app/sceneLayer/types/Types.h"

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
    const std::set<entt::entity>& GetSelectedEntities() const { return _selectedEntities; }
    void SetSelection(const std::set<entt::entity>& entities) { _selectedEntities = entities; }
    void AddToSelection(entt::entity entity) { _selectedEntities.insert(entity); }
    void RemoveFromSelection(entt::entity entity) { _selectedEntities.erase(entity); }
    void ClearSelection() { _selectedEntities.clear(); }
    bool IsSelected(entt::entity entity) const { return _selectedEntities.find(entity) != _selectedEntities.end(); }

    // === Edit Operations ===
    void Copy();
    void Cut();
    void Paste();
    void DeleteSelection();
    void Undo();
    void Redo();

    // === Scene Operations ===
    void SaveScene(const size_t sceneIndex);
    void SaveSceneAs(const size_t sceneIndex);
    void OpenScene();
    void RevertScene();
	void CloseScene(const size_t sceneIndex);
	bool IsSceneDirty(const size_t sceneIndex) const;

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

private:
    // State
    std::set<entt::entity> _selectedEntities;

    // Services (references to externally owned objects)
    SceneManager& _sceneManager;
    Core::EventBus& _eventBus;
    Core::Applicator<AppFieldTypes, AppComponentTypes>& _applicator;
    Core::UndoManager<AppFieldTypes>& _undoManager;
    void* _windowHandle;
    PopupManager* _popupManager = nullptr;
};