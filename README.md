Sandbox is an ECS-style 3D editor framework designed to support common scene editing workflows.
It provides a lightweight foundation for building 3D editor applications, with built-in support for serialization, undo/redo management, and multi-file editing.

The framework is cross-platform (currently only tested on windows) and can run directly in the browser via Emscripten/WebAssembly.

Try it live: [Live demo](https://animatorjeroen.github.io/3d-editor/)

## Building

### Prerequisites

- [Visual Studio](https://visualstudio.microsoft.com/) (with C++ workload)
- [vcpkg](https://github.com/microsoft/vcpkg) (for Assimp dependency) (get's automatically installed with the provided `quick_setup.bat` script)
- Git

### Windows — Visual Studio

1. **Install dependencies and generate project files** using the quick setup script:
   ```bat
   premake\quick_setup.bat
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