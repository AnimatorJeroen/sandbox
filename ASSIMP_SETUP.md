# FBX Import Setup Instructions

## Overview
This project supports importing 3D models with **full skeletal animation support** using the Assimp library. Import FBX, GLTF, Collada and other formats with bones, skinning, and animation keyframes.

## Features

### Mesh Import
- Vertices, normals, and UV coordinates
- Triangle-based geometry
- Automatic optimization

### Skeletal Animation (NEW!)
- ? **Bone Hierarchy** - Complete skeletal structure with parent-child relationships
- ? **Skinning Data** - Bone weights (up to 4 bones per vertex)
- ? **Animation Clips** - Multiple animations per model
- ? **Keyframe Data** - Position, rotation, and scale keyframes
- ? **FBX Isolation** - Animation data stored in dedicated FBX components

### Supported Components
The importer creates these isolated FBX-specific components:
- `FBXSkeletonComponent` - Bone hierarchy and transforms
- `FBXSkinComponent` - Vertex-to-bone weight assignments
- `FBXAnimationComponent` - Animation clips with keyframe data

## Installing Assimp

### Option 1: Using vcpkg (Recommended)

1. Install vcpkg if you haven't already:git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat
2. Install Assimp:vcpkg install assimp:x64-windows
3. Integrate vcpkg with Visual Studio:vcpkg integrate install
### Option 2: Manual Installation

1. Download Assimp prebuilt binaries from: https://github.com/assimp/assimp/releases
   - Download the Windows SDK package (e.g., assimp-sdk-X.X.X-setup.exe)

2. Install Assimp to a directory (e.g., `C:\Program Files\Assimp`)

3. Copy the files to your project:
   - Copy `include/assimp/*` to `vendor/include/assimp/`
   - Copy `lib/x64/assimp-vc142-mt.lib` (or similar) to `vendor/libs/`
   - Copy `bin/x64/assimp-vc142-mt.dll` to your build output directory

### Option 3: Build from Source

1. Clone Assimp repository:git clone https://github.com/assimp/assimp.git
cd assimp
2. Build using CMake:mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF
cmake --build . --config Release
3. Copy the built files to your vendor directory

## Usage

Once Assimp is installed:

1. **Regenerate project files:**cd premake
premake5 vs2022
2. **Import models in the editor:**
   - Use `File > Import Model...` menu
   - Or press `Ctrl+I`
   - Select FBX, OBJ, GLTF, or other supported formats

3. **View imported data:**
   - Meshes appear as child entities with `MeshComponent`
   - Skeletons are stored in `<ModelName>_Skeleton` entity with `FBXSkeletonComponent`
   - Animations are stored in `<ModelName>_Animations` entity with `FBXAnimationComponent`
   - Skinned meshes have `FBXSkinComponent` with bone weights

## Supported Formats

### With Full Animation Support:
- FBX (.fbx) ? **Best for animations**
- GLTF/GLB (.gltf, .glb) ? **Modern format**
- Collada (.dae)

### Static Meshes Only:
- Wavefront OBJ (.obj)
- 3DS Max (.3ds)
- STL (.stl)

### Others (40+ formats):
- Blender (.blend)
- And many more!

## Import Options

The importer supports various options (modifiable in code):Core::MeshImporter::ImportOptions options;
options.importAnimations = true;      // Import animation clips
options.importSkeleton = true;        // Import bone hierarchy
options.importSkinning = true;        // Import bone weights
options.importMeshes = true;          // Import geometry
options.generateMissingNormals = true; // Auto-generate normals
## FBX Component Architecture

The animation data is **completely isolated** in FBX-specific components:

### FBXSkeletonComponentstruct FBXSkeletonComponent {
    std::vector<FBXBone> bones;  // Bone hierarchy
    std::string skeletonName;
};
### FBXSkinComponentstruct FBXSkinComponent {
    std::vector<std::array<FBXVertexWeight, 4>> vertexWeights;
    int skeletonEntityIndex;  // Reference to skeleton entity
};
### FBXAnimationComponentstruct FBXAnimationComponent {
    std::vector<FBXAnimationClip> clips;
    int activeClipIndex;
    double currentTime;
    bool isPlaying;
    bool loop;
};
## Example: Importing Animated FBX

1. Export your character from Blender/Maya/3ds Max as FBX
2. In the editor: `File > Import Model...` (or `Ctrl+I`)
3. Select your FBX file
4. The importer creates:
   - Root entity with model name
   - Mesh entities with geometry (`MeshComponent`)
   - Skeleton entity (`FBXSkeletonComponent`)
   - Animation entity (`FBXAnimationComponent`)
   - Skin components on meshes (`FBXSkinComponent`)

## Next Steps

The imported FBX data is ready for:
- Animation playback system
- Bone transformation updates
- Skinned mesh rendering with GPU skinning
- Animation blending and state machines

You can implement these features by reading the FBX component data and applying transformations to your rendering pipeline.

## Troubleshooting

**Error: Cannot find assimp headers**
- Make sure Assimp headers are in `vendor/include/assimp/`
- Or ensure vcpkg integration is working

**Linker
