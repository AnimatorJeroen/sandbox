#pragma once
#ifndef PLATFORM_WASM
#include <string>
#include <vector>
#include <memory>
#include <map>

// Forward declarations
class Entity;
class Scene;

// Forward declare Assimp types in global namespace
struct aiScene;
struct aiMesh;

namespace Utils {

/// <summary>
/// MeshImporter handles importing 3D models from various file formats
/// using Assimp library. Supports FBX, OBJ, GLTF, and many other formats.
/// Includes support for skeletal animation data.
/// </summary>
class MeshImporter
{
public:
    struct ImportOptions {
        bool importAnimations = true;      // Import animation data
        bool importSkeleton = true;        // Import bone hierarchy
        bool importSkinning = true;        // Import bone weights
        bool importMeshes = true;          // Import mesh geometry
        bool generateMissingNormals = true; // Generate normals if not present
        
        ImportOptions() = default;
    };

    MeshImporter() = default;
    ~MeshImporter() = default;

    /// <summary>
    /// Import a 3D model file and create entities in the scene
    /// </summary>
    /// <param name="filepath">Path to the model file (FBX, OBJ, etc.)</param>
    /// <param name="scene">Target scene to import into</param>
    /// <param name="parent">Optional parent entity for imported meshes</param>
    /// <param name="options">Import options for controlling what data to import</param>
    /// <returns>True if import succeeded, false otherwise</returns>
    Entity ImportModel(const std::string& filepath, Scene* scene, Entity* parent = nullptr, 
                     const ImportOptions& options = ImportOptions());

    /// <summary>
    /// Get list of supported file extensions
    /// </summary>
    static std::vector<std::string> GetSupportedExtensions();

    /// <summary>
    /// Get the last error message
    /// </summary>
    const std::string& GetLastError() const { return _lastError; }

    /// <summary>
    /// Get import statistics from the last import
    /// </summary>
    struct ImportStats {
        int meshCount = 0;
        int boneCount = 0;
        int animationCount = 0;
        int vertexCount = 0;
        int triangleCount = 0;
    };
    const ImportStats& GetLastImportStats() const { return _stats; }

private:
    std::string _lastError;
    ImportStats _stats;
    
    // Helper to extract skeleton from Assimp scene
    bool ProcessSkeleton(const ::aiScene* aiScene, Entity* skeletonEntity, Scene* scene);
    
    // Helper to extract animations from Assimp scene
    bool ProcessAnimations(const ::aiScene* aiScene, Entity* animationEntity, Entity* skeletonEntity, Scene* scene);

    // Helper to extract bone weights from Assimp mesh
    void ProcessBoneWeights(const ::aiMesh* mesh, Entity* meshEntity, 
                           const std::map<std::string, int>& boneNameToIndex);
};

} // namespace Core
#endif // !PLATFORM_WASM
