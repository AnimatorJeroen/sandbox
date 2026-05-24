Sandbox is an ECS-style 3D editor framework designed to support common scene editing workflows.
It provides a lightweight foundation for building 3D editor applications, with built-in support for serialization, undo/redo management, and multi-file editing.

The framework is cross-platform (currently only tested on windows) and can run directly in the browser via Emscripten/WebAssembly.

Try it live: [Live demo](https://animatorjeroen.github.io/3d-editor/)

## Building

### Prerequisites

- [vcpkg](https://github.com/microsoft/vcpkg) (for Assimp dependency) (get's automatically installed with the provided `quick_setup.bat` script)

### Windows — Visual Studio

1. **Install dependencies and generate project files** using the quick setup script:
   ```bat
   > premake\quick_setup.bat
   > premake\runPremake_visualStudio.bat
   ```
   This will install Assimp via vcpkg and generate `Sandbox.sln`.

2. **Open the solution:**
   ```
   Sandbox.sln
   ```

3. **Build and run** using Visual Studio (`F5`) or select **Build → Build Solution**.

   Output binaries are placed in `build/bin/Debug/` or `build/bin/Release/`.

> **Manual steps** (if you prefer not to use `quick_setup.bat`):
> ```bat
> premake\setup_assimp.bat
> premake\runPremake_visualStudio.bat
> ```

---

## Project Structure

```
source/
├── core/                   # Reusable framework — no app-specific logic
│   ├── applicator/         # Undo/redo system (command pattern)
│   │   └── ops/            # Individual undoable operations
│   ├── event/              # Core event system
│   ├── reflection/         # Runtime type reflection utilities used by serialization, undo/redo and structural operations
│   ├── renderer/           # rendering abstraction and a super barebones OpenGL implementation
│   └── serializer/         # Scene serialization / deserialization
│
├── app/                    # Application-specific implementation
│   ├── editorLayer/        # Editor UI, tools, and interaction logic
│   │   └── panels/         # Individual ImGui panels (component view, etc.)
│   ├── sceneLayer/         # Scene graph, entity management
│   │   ├── components/     # ECS components (transform, mesh, light, …)
│   │   └── types/          # Shared scene types
│   ├── event/              # App-level events (scene events, etc.)
│   ├── importer/           # Asset importing (FBX / Assimp pipeline)
│   └── loggingLayer/       # Runtime logging
│
└── tests/                  # Tests to create a specific runtime with specific update loop
```

`core` contains everything that can be reused across different editor implementations.
`app` contains the concrete 3D editor built on top of that foundation.
`app` uses `application layers`, which are self-contained modules that can be enabled/disabled independently.
Note that the assimp importer is not part of the core framework, but is implemented as a layer in the app, so it can be easily swapped out or removed if not needed.

---

## Dependencies

| Library | Purpose | Bundled |
|---|---|---|
| [EnTT](https://github.com/skypjack/entt) | Entity-component-system | ✅ |
| [Dear ImGui](https://github.com/ocornut/imgui) | Immediate-mode UI | ✅ |
| [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) | In-viewport transform gizmos | ✅ |
| [GLFW](https://www.glfw.org/) | Window & input | ✅ |
| [GLEW](https://glew.sourceforge.net/) | OpenGL extension loading | ✅ |
| [GLM](https://github.com/g-truc/glm) | Math (vectors, matrices) | ✅ |
| [libsndfile](https://libsndfile.github.io/libsndfile/) | Audio file I/O | ✅ |
| [Assimp](https://github.com/assimp/assimp) | 3D model importing (FBX, OBJ, …) | via vcpkg |

## Design motivation

When prototyping ideas for animation/simulation applications, I usually don't want to have to think about the low level systems that handle non-destructive editing and serialization.
I use this project as the starting point for creating bespoke editor applications. I didn't find existing frameworks that are both leightweight and support undo/redo and serialization out of the boxs.
Everything else is kept to a minimum intentionally. The renderer implementation is minimal and you would have to write/extend your own to do anything useful.


User editable data out of the box.
Creating new components is trivial. They will automatically become serializable and editable for non-destructive and (undo/redo) and structural (copy/paste) operations.
Edit operations are described as field ops and structural ops. Field ops operate on basic types, (i.e vector3 Position) and structural ops tackle the more complicated problem
of adding/removing entities and storing relationships (such as child, parent). These relationships are resolved using UUID, which is the only component that is mandatory for the core functionality to work.

To add a component:
1. define it in sceneLayer/components/Compontents.h (or whereever you define your components)

2. To register it for undo/redo applicator (structural snapshots):
    - add the type to the "AppComponentTypes" tuple found in Compoenents.h
    - if it contains new field types that should be supported for field ops, add these in sceneLayer/types/Types.h (and add to the "AppFieldTypes" tuple)

3. To register it for serialization:
    - add Serialize and Deserialize functions in components/ComponentSerialization.h. Basic fields will be automatically recognized and serialized, but you need to handle more complex types (such as std::vector) yourself. See the examples in ComponentSerialization.h for reference.