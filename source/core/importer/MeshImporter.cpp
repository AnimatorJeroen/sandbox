#include "pch.h"
#include "MeshImporter.h"
#include "app/sceneLayer/Scene.h"
#include "app/sceneLayer/Entity.h"
#include "app/sceneLayer/components/Components.h"
#include "core/Logger.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <map>
#include <functional>

namespace Core {

bool MeshImporter::ImportModel(const std::string& filepath, Scene* scene, Entity parent, 
                               const ImportOptions& options)
{
    if (!scene)
    {
        _lastError = "Invalid scene pointer";
        LOG_ERROR() << "MeshImporter: " << _lastError;
        return false;
    }

    _stats = ImportStats();
    LOG_INFO() << "Importing model: " << filepath;

    Assimp::Importer importer;
    unsigned int postProcessFlags = 
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType |
        aiProcess_OptimizeMeshes |
        aiProcess_ValidateDataStructure;

    if (options.generateMissingNormals)
        postProcessFlags |= aiProcess_GenNormals;

    const ::aiScene* aiScene = importer.ReadFile(filepath, postProcessFlags);

    if (!aiScene || aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiScene->mRootNode)
    {
        _lastError = importer.GetErrorString();
        LOG_ERROR() << "Assimp import error: " << _lastError;
        return false;
    }

    LOG_INFO() << "Model loaded: " << aiScene->mNumMeshes << " meshes, " << aiScene->mNumAnimations << " animations";

    std::string filename = filepath;
    size_t lastSlash = filename.find_last_of("/\\");
    if (lastSlash != std::string::npos)
        filename = filename.substr(lastSlash + 1);
    size_t lastDot = filename.find_last_of('.');
    if (lastDot != std::string::npos)
        filename = filename.substr(0, lastDot);

    Entity rootEntity = scene->CreateEntity(filename);
    if (parent)
        scene->SetParent(rootEntity, parent);

    Entity skeletonEntity;
    std::map<std::string, int> boneNameToIndex;
    
    if (options.importSkeleton && aiScene->mNumMeshes > 0)
    {
        bool hasSkeleton = false;
        for (unsigned int i = 0; i < aiScene->mNumMeshes; i++)
        {
            if (aiScene->mMeshes[i]->HasBones())
            {
                hasSkeleton = true;
                break;
            }
        }

        if (hasSkeleton)
        {
            skeletonEntity = scene->CreateEntity(filename + "_Skeleton");
            scene->SetParent(skeletonEntity, rootEntity);
            
            if (ProcessSkeleton(aiScene, skeletonEntity))
            {
                auto& skeletonComp = skeletonEntity.GetComponent<FBXSkeletonComponent>();
                for (size_t i = 0; i < skeletonComp.bones.size(); i++)
                    boneNameToIndex[skeletonComp.bones[i].name] = static_cast<int>(i);
                _stats.boneCount = static_cast<int>(skeletonComp.bones.size());
            }
        }
    }

    if (options.importAnimations && aiScene->mNumAnimations > 0 && skeletonEntity)
    {
        Entity animEntity = scene->CreateEntity(filename + "_Animations");
        scene->SetParent(animEntity, rootEntity);
        
        if (ProcessAnimations(aiScene, animEntity))
            _stats.animationCount = aiScene->mNumAnimations;
    }

    if (options.importMeshes)
    {
        for (unsigned int i = 0; i < aiScene->mNumMeshes; i++)
        {
            aiMesh* mesh = aiScene->mMeshes[i];
            std::string meshName = mesh->mName.C_Str();
            if (meshName.empty())
                meshName = filename + "_Mesh_" + std::to_string(i);

            Entity meshEntity = scene->CreateEntity(meshName);
            scene->SetParent(meshEntity, rootEntity);

            auto& meshComponent = meshEntity.AddComponent<MeshComponent>();
            meshComponent.filepath = filepath;

            meshComponent.vertices.reserve(mesh->mNumVertices);
            for (unsigned int v = 0; v < mesh->mNumVertices; v++)
                meshComponent.vertices.push_back(vec3(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z));

            if (mesh->HasNormals())
            {
                meshComponent.normals.reserve(mesh->mNumVertices);
                for (unsigned int v = 0; v < mesh->mNumVertices; v++)
                    meshComponent.normals.push_back(vec3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z));
            }

            if (mesh->HasTextureCoords(0))
            {
                meshComponent.texCoords.reserve(mesh->mNumVertices);
                for (unsigned int v = 0; v < mesh->mNumVertices; v++)
                    meshComponent.texCoords.push_back(vec2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y));
            }

            for (unsigned int f = 0; f < mesh->mNumFaces; f++)
            {
                ::aiFace face = mesh->mFaces[f];
                for (unsigned int idx = 0; idx < face.mNumIndices; idx++)
                    meshComponent.indices.push_back(face.mIndices[idx]);
            }

            _stats.meshCount++;
            _stats.vertexCount += mesh->mNumVertices;
            _stats.triangleCount += mesh->mNumFaces;

            if (options.importSkinning && mesh->HasBones() && skeletonEntity)
                ProcessBoneWeights(mesh, meshEntity, boneNameToIndex);

            LOG_DEBUG() << "  Mesh " << i << " (" << meshName << "): " << meshComponent.vertices.size() << " verts, " 
                        << meshComponent.indices.size() / 3 << " tris, " << mesh->mNumBones << " bones";
        }
    }

    LOG_INFO() << "Import complete: " << filename << " (" << _stats.meshCount << " meshes, " 
               << _stats.boneCount << " bones, " << _stats.animationCount << " animations)";
    return true;
}

bool MeshImporter::ProcessSkeleton(const aiScene* aiScene, Entity skeletonEntity)
{
    if (!aiScene || !skeletonEntity)
        return false;

    auto& skeletonComp = skeletonEntity.AddComponent<FBXSkeletonComponent>();
    skeletonComp.skeletonName = skeletonEntity.GetComponent<NameComponent>().name.to_string();

    std::map<std::string, FBXBone> uniqueBones;

    for (unsigned int m = 0; m < aiScene->mNumMeshes; m++)
    {
        aiMesh* mesh = aiScene->mMeshes[m];
        if (!mesh->HasBones())
            continue;

        for (unsigned int b = 0; b < mesh->mNumBones; b++)
        {
            aiBone* bone = mesh->mBones[b];
            std::string boneName = bone->mName.C_Str();

            if (uniqueBones.find(boneName) == uniqueBones.end())
            {
                FBXBone fbxBone;
                fbxBone.name = boneName;

                const aiMatrix4x4& offset = bone->mOffsetMatrix;
                fbxBone.offsetMatrix = mat4(
                    offset.a1, offset.b1, offset.c1, offset.d1,
                    offset.a2, offset.b2, offset.c2, offset.d2,
                    offset.a3, offset.b3, offset.c3, offset.d3,
                    offset.a4, offset.b4, offset.c4, offset.d4
                );

                uniqueBones[boneName] = fbxBone;
            }
        }
    }

    std::function<void(aiNode*, int)> buildHierarchy = [&](aiNode* node, int parentIdx) {
        std::string nodeName = node->mName.C_Str();
        
        auto it = uniqueBones.find(nodeName);
        if (it != uniqueBones.end())
        {
            FBXBone& bone = it->second;
            bone.parentIndex = parentIdx;

            const aiMatrix4x4& transform = node->mTransformation;
            bone.localTransform = mat4(
                transform.a1, transform.b1, transform.c1, transform.d1,
                transform.a2, transform.b2, transform.c2, transform.d2,
                transform.a3, transform.b3, transform.c3, transform.d3,
                transform.a4, transform.b4, transform.c4, transform.d4
            );

            int currentIdx = static_cast<int>(skeletonComp.bones.size());
            skeletonComp.bones.push_back(bone);

            for (unsigned int i = 0; i < node->mNumChildren; i++)
                buildHierarchy(node->mChildren[i], currentIdx);
        }
        else
        {
            for (unsigned int i = 0; i < node->mNumChildren; i++)
                buildHierarchy(node->mChildren[i], parentIdx);
        }
    };

    buildHierarchy(aiScene->mRootNode, -1);

    LOG_INFO() << "Skeleton processed: " << skeletonComp.bones.size() << " bones";
    return true;
}

bool MeshImporter::ProcessAnimations(const ::aiScene* aiScene, Entity animationEntity)
{
    if (!aiScene || !animationEntity || aiScene->mNumAnimations == 0)
        return false;

    auto& animComp = animationEntity.AddComponent<FBXAnimationComponent>();

    for (unsigned int a = 0; a < aiScene->mNumAnimations; a++)
    {
        aiAnimation* anim = aiScene->mAnimations[a];
        
        FBXAnimationClip clip;
        clip.name = anim->mName.C_Str();
        if (clip.name.empty())
            clip.name = "Animation_" + std::to_string(a);
        
        clip.duration = anim->mDuration;
        clip.ticksPerSecond = anim->mTicksPerSecond != 0.0 ? anim->mTicksPerSecond : 25.0;

        for (unsigned int c = 0; c < anim->mNumChannels; c++)
        {
            aiNodeAnim* nodeAnim = anim->mChannels[c];
            
            FBXAnimationChannel channel;
            channel.boneName = nodeAnim->mNodeName.C_Str();

            for (unsigned int k = 0; k < nodeAnim->mNumPositionKeys; k++)
            {
                const aiVectorKey& key = nodeAnim->mPositionKeys[k];
                channel.positionKeys.emplace_back(key.mTime, vec3(key.mValue.x, key.mValue.y, key.mValue.z));
            }

            for (unsigned int k = 0; k < nodeAnim->mNumRotationKeys; k++)
            {
                const aiQuatKey& key = nodeAnim->mRotationKeys[k];
                channel.rotationKeys.emplace_back(key.mTime, glm::quat(key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z));
            }

            for (unsigned int k = 0; k < nodeAnim->mNumScalingKeys; k++)
            {
                const aiVectorKey& key = nodeAnim->mScalingKeys[k];
                channel.scaleKeys.emplace_back(key.mTime, vec3(key.mValue.x, key.mValue.y, key.mValue.z));
            }

            clip.channels.push_back(channel);
        }

        animComp.clips.push_back(clip);
        
        LOG_INFO() << "  Animation '" << clip.name << "': " << clip.channels.size() << " channels, duration: " << clip.duration << " ticks";
    }

    return true;
}

void MeshImporter::ProcessBoneWeights(const ::aiMesh* mesh, Entity meshEntity, 
                                      const std::map<std::string, int>& boneNameToIndex)
{
    if (!mesh || !meshEntity || !mesh->HasBones())
        return;

    auto& skinComp = meshEntity.AddComponent<FBXSkinComponent>();
    skinComp.vertexWeights.resize(mesh->mNumVertices);

    for (auto& vw : skinComp.vertexWeights)
        for (int i = 0; i < 4; i++)
            vw[i] = FBXVertexWeight(-1, 0.0f);

    for (unsigned int b = 0; b < mesh->mNumBones; b++)
    {
        aiBone* bone = mesh->mBones[b];
        std::string boneName = bone->mName.C_Str();

        auto it = boneNameToIndex.find(boneName);
        if (it == boneNameToIndex.end())
        {
            LOG_WARN() << "Bone '" << boneName << "' not found in skeleton";
            continue;
        }

        int boneIndex = it->second;

        for (unsigned int w = 0; w < bone->mNumWeights; w++)
        {
            unsigned int vertexId = bone->mWeights[w].mVertexId;
            float weight = bone->mWeights[w].mWeight;

            if (vertexId >= skinComp.vertexWeights.size())
                continue;

            auto& weights = skinComp.vertexWeights[vertexId];
            int insertIdx = -1;
            float minWeight = 1.0f;

            for (int i = 0; i < 4; i++)
            {
                if (weights[i].boneIndex == -1)
                {
                    insertIdx = i;
                    break;
                }
                if (weights[i].weight < minWeight)
                {
                    minWeight = weights[i].weight;
                    insertIdx = i;
                }
            }

            if (insertIdx != -1 && weight > minWeight)
                weights[insertIdx] = FBXVertexWeight(boneIndex, weight);
        }
    }

    for (auto& weights : skinComp.vertexWeights)
    {
        float totalWeight = 0.0f;
        for (int i = 0; i < 4; i++)
            if (weights[i].boneIndex != -1)
                totalWeight += weights[i].weight;

        if (totalWeight > 0.0f)
            for (int i = 0; i < 4; i++)
                if (weights[i].boneIndex != -1)
                    weights[i].weight /= totalWeight;
    }

    LOG_DEBUG() << "  Processed skinning weights for " << mesh->mNumVertices << " vertices";
}

std::vector<std::string> MeshImporter::GetSupportedExtensions()
{
    return {
        "fbx", "obj", "dae", "gltf", "glb", "blend", 
        "3ds", "ase", "ifc", "xgl", "zgl", "ply", 
        "dxf", "lwo", "lws", "lxo", "stl", "x", "ac", 
        "ms3d", "cob", "scn", "mesh", "mesh.xml"
    };
}

} // namespace Core
