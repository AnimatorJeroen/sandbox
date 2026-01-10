# Modifier Keys Implementation

## Overview
Modifier keys (Shift, Ctrl, Alt, Super) are tracked directly in key events without requiring an additional service.

## Solution
The modifier flags from GLFW are passed through `KeyDownEvent` and `KeyUpEvent`, allowing any layer to check which modifiers were pressed at the time of the key event.

## Usage Example

```cpp
bool YourLayer::OnKeyDownEvent(const Core::KeyDownEvent& e)
{
    // Check for Ctrl+S
    if (e.key == 'S' && (e.mods & Core::MOD_CONTROL)) {
        SaveFile();
    }
    
    // Check for Ctrl+Shift+Z (Redo)
    if (e.key == 'Z' && (e.mods & Core::MOD_CONTROL) && (e.mods & Core::MOD_SHIFT)) {
        Redo();
    }
    
    // Check for Ctrl+Z (Undo)
    else if (e.key == 'Z' && (e.mods & Core::MOD_CONTROL)) {
        Undo();
    }
    
    return false;
}
```

## Modifier Constants (defined in `core/event/KeyEvent.h`)
- `Core::MOD_SHIFT` - 0x0001
- `Core::MOD_CONTROL` - 0x0002
- `Core::MOD_ALT` - 0x0004
- `Core::MOD_SUPER` - 0x0008
- `Core::MOD_CAPS_LOCK` - 0x0010
- `Core::MOD_NUM_LOCK` - 0x0020

## Benefits
1. **No service bloat**: No additional services to manage
2. **Direct and simple**: Modifier state is directly in the event
3. **Event-specific**: Know exactly which modifiers were pressed when the key was pressed
4. **Familiar pattern**: Standard bitflag checking used in many frameworks

## Implementation Details
- Modified `KeyDownEvent` and `KeyUpEvent` to include `int mods` field
- Window class captures `mods` parameter from GLFW callbacks and passes it to events
- All layers can check modifiers using bitwise AND operations
