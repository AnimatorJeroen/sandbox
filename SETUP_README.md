# Sandbox Project Setup

## Quick Start (Recommended)

**For first-time setup, just run:**

```bash
quick_setup.bat
```

This will:
1. ? Install vcpkg (if not already installed)
2. ? Install Assimp via vcpkg
3. ? Integrate with Visual Studio
4. ? Generate project files

Then open `Sandbox.sln` and build!

---

## Manual Setup

### Option 1: Automated Setup (Easy)

**Step 1: Install Dependencies**
```bash
setup_assimp.bat
```

**Step 2: Generate Project**
```bash
cd premake
premake5 vs2022
```

### Option 2: Manual Installation

**Step 1: Install vcpkg**
```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat
```

**Step 2: Install Assimp**
```bash
vcpkg install assimp:x64-windows
vcpkg integrate install
```

**Step 3: Generate Project**
```bash
cd premake
premake5 vs2022
```

---

## Scripts Overview

### `quick_setup.bat` 
**One-command full setup**
- Installs all dependencies
- Generates project files
- Ready to build immediately

### `setup_assimp.bat`
**Dependency installation only**
- Checks for existing vcpkg installation
- Installs vcpkg if needed
- Installs Assimp
- Integrates with Visual Studio

### `premake5.lua`
**Project generation with dependency checking**
- Detects vcpkg installation
- Shows warnings if dependencies missing
- Displays helpful setup messages
- Automatically configures include paths

---

## Build Configurations

### Debug
- Symbols enabled
- Edit & Continue support
- No optimization
- Best for development

### Release
- Optimized
- No debug symbols
- Best for production

---

## Features

### FBX Model Import (Ctrl+I)
Once built, you can import:
- **FBX** - Full skeletal animation support
- **GLTF/GLB** - Modern 3D format
- **OBJ** - Static meshes
- **Collada (DAE)** - Animations
- **40+ other formats** via Assimp

### Imported Data
- ? Mesh geometry (vertices, normals, UVs)
- ? Skeletal hierarchies
- ? Animation clips with keyframes
- ? Bone weights for skinning
- ? Multiple meshes per model

---

## Project Structure

```
sandbox/
??? build/                   # Generated build files
??? source/                  # C++ source code
?   ??? app/                # Application layer
?   ?   ??? editorLayer/   # Editor UI & tools
?   ?   ??? sceneLayer/    # Scene management & ECS
?   ??? core/              # Core engine systems
?       ??? importer/      # MeshImporter (Assimp)
?       ??? renderer/      # OpenGL renderer
?       ??? applicator/    # Undo/redo system
??? vendor/                 # Third-party libraries
?   ??? include/           # Headers (glm, entt, imgui, assimp)
?   ??? libs/              # Prebuilt libraries
??? premake/               # Build system
?   ??? premake5.lua       # Project configuration
??? quick_setup.bat        # One-command setup
??? setup_assimp.bat       # Dependency installer
??? ASSIMP_SETUP.md        # Detailed Assimp guide
```

---

## Troubleshooting

### "vcpkg not found"
**Solution:** Run `setup_assimp.bat` - it will install vcpkg automatically

### "Cannot open include file: 'assimp/Importer.hpp'"
**Solution:** Assimp not installed. Run:
```bash
vcpkg install assimp:x64-windows
vcpkg integrate install
```

### "LNK2019: unresolved external symbol" (Assimp functions)
**Solution:** vcpkg integration not working. Run:
```bash
vcpkg integrate install
```
Then regenerate project:
```bash
cd premake
premake5 vs2022
```

### Build is slow
**Solution:** Already enabled! Project uses `/MP` flag for multiprocessor compilation.

### Git not installed (for vcpkg)
**Solution:** Install Git from https://git-scm.com/downloads

---

## Dependencies

### Required
- **Visual Studio 2022** (or 2019)
- **Windows 10/11**

### Automatically Installed via vcpkg
- **Assimp** - 3D model import (FBX, OBJ, GLTF, etc.)

### Included in vendor/
- **GLFW** - Window management
- **GLEW** - OpenGL extensions
- **GLM** - Math library
- **EnTT** - Entity Component System
- **ImGui** - UI framework
- **ImGuizmo** - 3D gizmos

---

## Usage After Setup

### Building
1. Open `Sandbox.sln`
2. Select Debug or Release configuration
3. Press F7 to build
4. Press F5 to run with debugger (or Ctrl+F5 without)

### Importing Models
1. Run the editor
2. Create or open a scene
3. Press **Ctrl+I** or use **File > Import Model...**
4. Select FBX/OBJ/GLTF file
5. Model appears in scene hierarchy with:
   - Meshes
   - Skeleton (if present)
   - Animations (if present)
   - Bone weights

### Scene Management
- **Ctrl+N** - New scene
- **Ctrl+S** - Save scene
- **Ctrl+O** - Open scene
- **Ctrl+Z** - Undo
- **Ctrl+Shift+Z** - Redo
- **Ctrl+C/X/V** - Copy/Cut/Paste entities

---

## Documentation

- **`ASSIMP_SETUP.md`** - Detailed Assimp installation guide
- **`FBX_IMPLEMENTATION_SUMMARY.md`** - FBX import feature overview
- **`MESHIMPORTER_COMPLETE.md`** - Technical implementation details

---

## Support

### Issues
If you encounter problems:
1. Check `ASSIMP_SETUP.md` for detailed troubleshooting
2. Ensure vcpkg integrated: `vcpkg integrate install`
3. Try clean rebuild: Delete `build/` folder and regenerate

### Build Times
- **First build**: 5-10 minutes (includes Assimp compilation by vcpkg)
- **Subsequent builds**: 30-60 seconds
- **Incremental builds**: 5-10 seconds

---

## What's Next?

After successful setup and build:

1. **Import a test model** - Try importing an FBX with animations
2. **Explore the scene hierarchy** - See the imported mesh/skeleton entities
3. **Check component data** - View mesh vertices, bones, animations in properties
4. **Implement animation playback** - Use the imported FBX data to animate models
5. **Build a rendering system** - Render the imported meshes with OpenGL

See `FBX_IMPLEMENTATION_SUMMARY.md` for ideas on building animation systems!

---

**Happy Coding! ??**
