# Improved Shared Resource Architecture

## Overview

This document describes the enhanced architecture for managing shared resources (like Scene, EventBus, etc.) in a safe and maintainable way.

## Key Components

### 1. **LayerContext with Access Control**

The `LayerContext` now includes:
- **Thread Safety**: Mutex protection for all operations
- **Access Policies**: `ReadOnly` vs `ReadWrite` permissions
- **Validation**: Prevents unauthorized modifications

```cpp
// In Application setup (has ReadWrite access):
auto sceneManager = std::make_shared<SceneManager>();
context.Register<SceneManager>(sceneManager, ResourceAccessPolicy::ReadOnly);

// In layers (ReadOnly access):
auto sceneManager = context.Get<SceneManager>(); // Can read
// context.Register<SceneManager>(...); // Would throw exception!
```

### 2. **SceneManager - The Indirection Layer**

`SceneManager` provides:
- **Multi-scene support** (tabs)
- **Active scene management**
- **Load/Save operations**
- **Callback notifications** when scenes change
- **Safe scene switching**

```cpp
// Usage example:
auto sceneManager = context.Get<SceneManager>();

// Get active scene
auto activeScene = sceneManager->GetActiveScene();

// Work with scene safely
activeScene->GetShapes().push_back(newShape);

// Load a new scene (manager handles the switch)
sceneManager->LoadScene("path/to/scene.dat", true);

// Create new scene
auto newScene = sceneManager->CreateNewScene("MyScene", false);

// Switch between scenes
sceneManager->SetActiveScene(1); // Switch to second tab
```

### 3. **Callback System for Scene Changes**

Instead of manually updating references, layers register callbacks:

```cpp
// In layer constructor:
auto sceneManager = _context.Get<SceneManager>();

sceneManager->RegisterSceneChangedCallback([this](std::shared_ptr<Scene> newScene) {
    // Update local references
    _mySceneRef = newScene;
    _testScene.UpdateScene(newScene);
    // Refresh UI, etc.
});
```

## Architecture Benefits

### Safety
1. ✅ **No Unauthorized Mutations**: Only SceneManager can change scenes
2. ✅ **Thread Safe**: All LayerContext operations are mutex-protected
3. ✅ **No Stale References**: Callbacks ensure everyone stays updated
4. ✅ **Validation**: Access policies prevent misuse

### Scalability
1. ✅ **Multi-Scene Support**: Handle multiple open scenes (tabs)
2. ✅ **Extensible**: Easy to add more managed resources (TextureManager, AudioManager, etc.)
3. ✅ **Decoupled**: Layers don't directly manipulate shared state

### Maintainability
1. ✅ **Single Source of Truth**: SceneManager is the authority
2. ✅ **Clear Ownership**: Manager pattern makes responsibilities obvious
3. ✅ **Testable**: Can mock managers easily

## Recommended Patterns

### Pattern 1: Manager for Mutable Resources

For any resource that needs to change (Scene, Project, Document):
```cpp
class ResourceManager {
    std::shared_ptr<Resource> GetActive();
    void SetActive(std::shared_ptr<Resource>);
    void RegisterCallback(ResourceChangedCallback);
};

// Register as ReadOnly
context.Register<ResourceManager>(manager, ResourceAccessPolicy::ReadOnly);
```

### Pattern 2: Direct Registration for Immutable Singletons

For truly immutable resources (EventBus, Renderer):
```cpp
// These can be registered directly since they don't change
context.Register<EventBus>(eventBus, ResourceAccessPolicy::ReadOnly);
context.Register<Renderer>(renderer, ResourceAccessPolicy::ReadOnly);
```

### Pattern 3: Application-Level Control

Only `Application` class should:
- Create managers
- Register them in LayerContext
- Have ReadWrite access

Layers should:
- Only `Get()` from context
- Never `Register()` 
- Use manager APIs for changes

## Migration Guide

### Before (Unsafe):
```cpp
// Layer could do this:
_context.Register<Scene>(newScene); // Dangerous!
```

### After (Safe):
```cpp
// Layer uses manager:
auto sceneManager = _context.Get<SceneManager>();
sceneManager->LoadScene("path.dat"); // Safe!

// Or requests via event:
_eventBus.PushEvent<RequestLoadSceneEvent>("path.dat");
```

## Other Shared Resources

Apply the same pattern to:
- **TextureManager**: Manage texture loading/caching
- **AudioManager**: Handle audio resources
- **ProjectManager**: Project settings, recent files
- **AssetBrowser**: Asset database
- **HistoryManager**: Undo/Redo stack

## Example: Full Setup

```cpp
// In Application::Application()
{
    // Create managers
    auto sceneManager = std::make_shared<SceneManager>();
    auto eventBus = std::make_shared<EventBus>();
    
    // Register with appropriate access
    _layerContext.Register<SceneManager>(sceneManager, ResourceAccessPolicy::ReadOnly);
    _layerContext.Register<EventBus>(eventBus, ResourceAccessPolicy::ReadOnly);
    
    // Layers now get these via Get<>() and use their APIs
    PushLayer<UIApplicationLayer>();
    PushLayer<SceneApplicationLayer>();
}
```

## Summary

The improved architecture provides:
- **Safety** through access control and indirection
- **Flexibility** through manager pattern
- **Scalability** through callback system
- **Clarity** through single responsibility

This makes your codebase more maintainable and less error-prone as it grows.
