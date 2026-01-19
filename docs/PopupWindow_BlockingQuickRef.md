# Blocking Popup Quick Reference

## ShowBlockingConfirmation

### Signature
```cpp
bool PopupManager::ShowBlockingConfirmation(
    const std::string& title, 
    const std::string& message
)
```

### Description
Shows a modal confirmation dialog that **blocks execution** until the user responds. Returns `true` if user clicked "Yes", `false` if user clicked "No".

### When to Use
- When you need an immediate response before continuing
- When called outside the normal render loop
- For critical operations that require user confirmation (e.g., closing, deleting)

### Example Usage

```cpp
// Basic usage
auto* pm = _editorContext.GetPopupManager();
bool confirmed = pm->ShowBlockingConfirmation(
    "Close Scene",
    "Are you sure you want to close this scene?"
);

if (confirmed) {
    // User clicked Yes
    CloseScene();
}
```

```cpp
// With scene information
std::string message = "Close \"" + sceneName + "\"?\nUnsaved changes will be lost.";
bool confirmed = pm->ShowBlockingConfirmation("Close Scene", message);
```

```cpp
// Error checking
auto* pm = _editorContext.GetPopupManager();
if (!pm) {
    // Fallback behavior if popup manager not available
    return;
}

if (pm->ShowBlockingConfirmation("Delete", "Delete all items?")) {
    DeleteAllItems();
}
```

## Setup Required

### In EditorApplicationLayer Constructor
```cpp
// Set window reference for blocking popups
_popupManager.SetWindow(ctx.Get<Core::Window>().get());
```

Without this, `ShowBlockingConfirmation()` will return `false` immediately.

## Comparison with ShowConfirmation

| Feature | ShowConfirmation | ShowBlockingConfirmation |
|---------|-----------------|-------------------------|
| **Blocking** | No | Yes |
| **Returns immediately** | Yes | No |
| **Return value** | void | bool |
| **Callbacks** | Required | Not used |
| **Use case** | Async confirmation | Sync confirmation |

### ShowConfirmation (Non-blocking)
```cpp
pm->ShowConfirmation(
    "Title", 
    "Message",
    []() { /* On OK */ },
    []() { /* On Cancel */ }
);
// Continues immediately
```

### ShowBlockingConfirmation (Blocking)
```cpp
bool result = pm->ShowBlockingConfirmation("Title", "Message");
// Waits for user response
if (result) {
    // User clicked Yes
}
```

## Technical Details

### How It Works
1. Creates PopupWindow with Yes/No buttons
2. Enters mini render loop
3. Renders only the popup
4. Polls events and handles input
5. Returns when popup is closed

### Performance
- Minimal overhead (renders only popup, not full UI)
- No blocking of OS window messages
- Safe to use anywhere in code

### Limitations
- Only works if window reference is set
- Blocks main thread (by design)
- Cannot show progress during blocking

## Common Patterns

### Close with Confirmation
```cpp
void ShowCloseConfirmation(size_t sceneIndex) {
    auto* pm = _editorContext.GetPopupManager();
    if (!pm) return;
    
    std::string msg = "Close \"" + GetSceneName(sceneIndex) + "\"?";
    if (pm->ShowBlockingConfirmation("Close Scene", msg)) {
        CloseScene(sceneIndex);
    }
}
```

### Delete with Confirmation
```cpp
void DeleteWithConfirmation() {
    auto* pm = _editorContext.GetPopupManager();
    if (!pm) return;
    
    if (pm->ShowBlockingConfirmation("Delete", "Delete selected items?")) {
        _editorContext.DeleteSelection();
    }
}
```

### Revert with Confirmation
```cpp
void RevertWithConfirmation() {
    auto* pm = _editorContext.GetPopupManager();
    if (!pm) return;
    
    std::string msg = "Revert all changes?\nThis cannot be undone.";
    if (pm->ShowBlockingConfirmation("Revert", msg)) {
        _editorContext.RevertScene();
    }
}
```

## Error Handling

```cpp
auto* pm = _editorContext.GetPopupManager();
if (!pm) {
    LOG_ERROR() << "PopupManager not available";
    // Fallback behavior
    return;
}

bool result = pm->ShowBlockingConfirmation("Title", "Message");
// result is always valid (true or false)
```

## Best Practices

? **DO**
- Check if PopupManager is available
- Use for critical confirmations
- Keep messages clear and concise
- Set window reference in initialization

? **DON'T**
- Use for non-critical information (use ShowInfo instead)
- Call in performance-critical loops
- Forget to check the return value
- Use for long-running operations (it blocks!)

## Related Functions

- `ShowInfo()` - Non-blocking information popup
- `ShowWarning()` - Non-blocking warning popup
- `ShowConfirmation()` - Non-blocking confirmation with callbacks
- `ShowPopup()` - Custom non-blocking popup
