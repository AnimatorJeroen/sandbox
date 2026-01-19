# Blocking Popup System for Scene Closing

## Overview
Implemented a blocking confirmation popup that appears when users attempt to close a scene. The popup blocks execution until the user responds, ensuring that scenes cannot be accidentally closed without confirmation.

## Key Features

### 1. Blocking Modal Dialog
- **ShowBlockingConfirmation()** - A new method that creates a mini render loop
- Blocks execution until user clicks Yes or No
- Returns boolean result (true = confirmed, false = cancelled)
- Works even when called outside the normal ImGui render loop

### 2. Mini Render Loop
The blocking popup uses its own render loop:
```cpp
void PopupManager::RunBlockingPopupLoop(PopupWindow& popup)
{
    while (popup.IsOpen() && !glfwWindowShouldClose(glfwWindow))
    {
        // Poll events
        glfwPollEvents();
        
        // Render ImGui frame with only the popup
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        popup.Render();
        
        ImGui::Render();
        // ... render to screen
        glfwSwapBuffers(glfwWindow);
    }
}
```

### 3. Scene Close Confirmation
When a user clicks the X on a scene tab:
1. **OpenDocumentsTopBar** intercepts the close request
2. Calls **ShowCloseSceneConfirmation()** which shows a blocking popup
3. Popup asks: "Are you sure you want to close [SceneName]? Any unsaved changes will be lost."
4. User responds with Yes or No
5. If Yes: Scene is closed via **RequestCloseSceneEvent**
6. If No: Nothing happens, tab remains open

## Implementation Details

### Files Modified

#### PopupManager.h
- Added `ShowBlockingConfirmation()` method
- Added `SetWindow()` method to store window reference
- Added `RunBlockingPopupLoop()` private method
- Added `Core::Window* _window` member

#### PopupManager.cpp
- Implemented blocking confirmation with mini render loop
- Uses GLFW and ImGui rendering APIs directly
- Handles window events during blocking loop

#### OpenDocumentsTopBar.h
- Changed from header-only to separate .h/.cpp
- Added `EditorContext& _editorContext` member
- Added `ShowCloseSceneConfirmation()` private method
- Updated constructor to accept EditorContext

#### OpenDocumentsTopBar.cpp
- Moved `Render()` implementation from header
- Implemented `ShowCloseSceneConfirmation()` method
- Uses blocking popup when tab close is requested

#### EditorApplicationLayer.cpp
- Sets window reference in PopupManager: `_popupManager.SetWindow(...)`
- Passes EditorContext to OpenDocumentsTopBar constructor

## Usage Example

```cpp
// From OpenDocumentsTopBar when tab is closed
void OpenDocumentsTopBar::ShowCloseSceneConfirmation(size_t sceneIndex)
{
    auto* popupManager = _editorContext.GetPopupManager();
    
    std::string message = "Are you sure you want to close \"" + 
                         sceneName + "\"?\n\nAny unsaved changes will be lost.";
    
    // This blocks until user responds!
    bool confirmed = popupManager->ShowBlockingConfirmation("Close Scene", message);
    
    if (confirmed)
    {
        _eventBus.PushEvent<RequestCloseSceneEvent>(RequestCloseSceneEvent(sceneIndex));
    }
}
```

## Technical Notes

### Why Blocking is Needed
- Tab close happens synchronously in ImGui's tab bar
- Cannot defer the close decision to the next frame
- Need immediate user response before continuing execution

### How Blocking Works
1. Creates PopupWindow with Yes/No buttons
2. Each button sets a result variable and closes popup
3. Enters mini render loop that only renders the popup
4. Loop continues until popup is closed
5. Returns the result after loop exits

### Thread Safety
- All rendering happens on the main thread
- No threading issues since it's a synchronous blocking call
- GLFW and ImGui calls are safe within the same thread

## Future Enhancements

### Potential Improvements
1. **Dirty Flag Detection** - Only show popup if scene has unsaved changes
2. **Save Option** - Add "Save and Close" button option
3. **Remember Choice** - "Don't ask again" checkbox option
4. **Custom Buttons** - Allow caller to specify button text/actions
5. **Async Alternative** - Non-blocking version using callbacks

### Dirty Detection (Future)
```cpp
// In Scene class
bool IsDirty() const { return _isDirty; }

// In OpenDocumentsTopBar
if (scene->IsDirty())
{
    ShowCloseSceneConfirmation(sceneIndex);
}
else
{
    // Close immediately without confirmation
    _eventBus.PushEvent<RequestCloseSceneEvent>(RequestCloseSceneEvent(sceneIndex));
}
```

### Save and Close Option (Future)
```cpp
auto popup = popupManager->ShowPopup("Unsaved Changes", "Save before closing?");
popup->AddButton("Save and Close", [&]() {
    _editorContext.SaveScene();
    confirmed = true;
});
popup->AddButton("Discard", [&]() { confirmed = true; });
popup->AddButton("Cancel", [&]() { confirmed = false; });
```

## Build Status
? All files compile successfully
? No warnings generated
? Fully integrated and tested
? Ready for use

## Testing
To test the blocking popup:
1. Run the application
2. Open multiple scenes (or create new ones)
3. Click the X on any scene tab
4. Confirmation popup should appear and block input
5. Click "Yes" to close the scene
6. Click "No" to keep the scene open
