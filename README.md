## About

Sandbox is an ECS-style 3D editor framework designed to support common scene editing workflows.
It provides a lightweight foundation for building 3D editor applications, with built-in support for serialization, undo/redo management, and multi-file editing.

The framework is cross-platform (currently only tested on windows) and can run directly in the browser via Emscripten/WebAssembly.

Try it: [Live demo](https://animatorjeroen.github.io/3d-editor/)

## Design motivation

When prototyping animation and simulation tools, I kept running into the same problem: no lightweight framework that provides undo/redo and serialization out of the box without a lot of overhead. This project is the starting point I use for building bespoke editor applications.

Everything outside of that core goal is intentionally kept minimal. The renderer is a bare-bones OpenGL implementation — you are expected to write or extend your own for anything beyond basic scene display.

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


### Edit operations

Components are automatically serializable and support both types of edit operations:

- **Field ops** — operate on individual fields of basic types (e.g. a `vec3` position). Supports undo/redo at the field level.
- **Structural ops** — handle adding/removing entities and storing relationships (e.g parent/child). Relationships are resolved by UUID, which is the only component required by the core framework.

### Adding a new component

**1. Define it**

Add your struct in `source/app/sceneLayer/components/Components.h` (or where your components are defined)

**2. Register it for undo/redo**

- Add the type to the `AppComponentTypes` tuple in `Components.h`.
- If the component introduces new field types, add them to the `AppFieldTypes` tuple in `source/app/sceneLayer/types/Types.h`.

**3. Register it for serialization**

Add `Serialize` and `Deserialize` functions in `source/app/sceneLayer/components/ComponentSerialization.h`.
Basic fields are recognized and serialized automatically. Complex types (e.g. `std::vector`) require manual handling — see the existing entries in that file for reference.
