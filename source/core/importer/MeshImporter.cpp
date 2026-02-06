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

    static glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
    {
        glm::mat4 to;
        //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
        to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
        to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
        to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
        to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
        return to;
    }

bool MeshImporter::ImportModel(const std::string& filepath, Scene* scene, Entity* parent, 
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
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
    unsigned int postProcessFlags = 
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType |
        aiProcess_OptimizeMeshes |
        aiProcess_ValidateDataStructure |
        aiProcess_LimitBoneWeights; // Limit to 4 bones per vertex

    if (options.generateMissingNormals)
        postProcessFlags |= aiProcess_GenNormals;
    
    // CRITICAL for FBX skeletal animations: PopulateArmatureData tells Assimp to properly 
    // set up bone hierarchy and transforms from the armature/skeleton data
    if (options.importSkeleton || options.importAnimations)
    {
        //postProcessFlags |= aiProcess_PopulateArmatureData;
        LOG_DEBUG() << "Using aiProcess_PopulateArmatureData for skeleton/animation import";
    }

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
        scene->SetParent(rootEntity, *parent);

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
            
            if (ProcessSkeleton(aiScene, &skeletonEntity))
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
        
        if (ProcessAnimations(aiScene, &animEntity, &skeletonEntity))
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
                ProcessBoneWeights(mesh, &meshEntity, boneNameToIndex);

            LOG_DEBUG() << "  Mesh " << i << " (" << meshName << "): " << meshComponent.vertices.size() << " verts, " 
                        << meshComponent.indices.size() / 3 << " tris, " << mesh->mNumBones << " bones";
        }
    }

    LOG_INFO() << "Import complete: " << filename << " (" << _stats.meshCount << " meshes, " 
               << _stats.boneCount << " bones, " << _stats.animationCount << " animations)";
    return true;
}

bool MeshImporter::ProcessSkeleton(const aiScene* aiScene, Entity* skeletonEntity)
{
    if (!aiScene || !skeletonEntity)
        return false;

    auto& skeletonComp = skeletonEntity->AddComponent<FBXSkeletonComponent>();
    skeletonComp.skeletonName = skeletonEntity->GetComponent<NameComponent>().name.to_string();

    // Map aiBone name to bone data (offsetMatrix, etc.)
    std::map<std::string, FBXBone> boneDataByAiBoneName;

    for (unsigned int m = 0; m < aiScene->mNumMeshes; m++)
    {
        aiMesh* mesh = aiScene->mMeshes[m];
        if (!mesh->HasBones())
            continue;

        for (unsigned int b = 0; b < mesh->mNumBones; b++)
        {
            aiBone* bone = mesh->mBones[b];
            std::string aiBoneName = bone->mName.C_Str();

            if (boneDataByAiBoneName.find(aiBoneName) == boneDataByAiBoneName.end())
            {
                FBXBone fbxBone;

                fbxBone.name = aiBoneName;
                
                const aiMatrix4x4& offset = bone->mOffsetMatrix;
                fbxBone.offsetMatrix = ConvertMatrixToGLMFormat(offset);
                
                // Initialize animatedTransform to identity (will be updated by FbxPlayer)
                fbxBone.animatedTransform = mat4(1.0f);

                boneDataByAiBoneName[aiBoneName] = fbxBone;
            }
        }
    }

    std::function<void(aiNode*, int)> buildHierarchy = [&](aiNode* node, int parentIdx) {
        std::string nodeName = node->mName.C_Str();
        
        auto it = boneDataByAiBoneName.find(nodeName);
        if (it != boneDataByAiBoneName.end())
        {
            FBXBone& bone = it->second;

            bone.parentIndex = parentIdx;

            const aiMatrix4x4& transform = node->mTransformation;
            //bone.localTransform = ConvertMatrixToGLMFormat(transform);

            // Initialize animatedTransform to the bind pose local transform
            bone.animatedTransform = bone.localTransform;

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

    // Build child indices for all bones
    for (size_t i = 0; i < skeletonComp.bones.size(); i++)
    {
        FBXBone& bone = skeletonComp.bones[i];
        // Find all children of this bone
        for (size_t j = 0; j < skeletonComp.bones.size(); j++)
        {
            if (skeletonComp.bones[j].parentIndex == static_cast<int>(i))
            {
                bone.childIndices.push_back(static_cast<int>(j));
            }
        }
    }

    for (auto& bone : skeletonComp.bones)
    {
        mat4 parWorldM = glm::mat4(1.0f);
        if (bone.parentIndex != -1)
            parWorldM = skeletonComp.bones[bone.parentIndex].offsetMatrix;
        bone.localRestTransform = parWorldM * glm::inverse(bone.offsetMatrix);
    }

    LOG_INFO() << "Skeleton processed: " << skeletonComp.bones.size() << " bones";
    return true;
}

bool MeshImporter::ProcessAnimations(const ::aiScene* aiScene, Entity* animationEntity, Entity* skeletonEntity)
{
    if (!aiScene || !animationEntity || aiScene->mNumAnimations == 0)
        return false;

    auto& skeletonComp = skeletonEntity->GetComponent<FBXSkeletonComponent>();

    auto& animComp = animationEntity->AddComponent<FBXAnimationComponent>();

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
            std::string boneName = nodeAnim->mNodeName.C_Str();

            for (size_t i = 0; i < skeletonComp.bones.size(); i++)
            {
                if (std::strcmp(skeletonComp.bones[i].name.data, boneName.data()) == 0)
                {
                    channel.boneIndex = static_cast<int>(i);
                    break;
                }
            }
        
            for (unsigned int k = 0; k < nodeAnim->mNumPositionKeys; k++)
            {
                const aiVectorKey& key = nodeAnim->mPositionKeys[k];
                channel.positionKeys.emplace_back(key.mTime, vec3(key.mValue.x, key.mValue.y, key.mValue.z));
            }

            for (unsigned int k = 0; k < nodeAnim->mNumRotationKeys; k++)
            {
                const aiQuatKey& key = nodeAnim->mRotationKeys[k];
                // Assimp quaternion is (w, x, y, z), assign directly
                glm::quat rotation;
                rotation.w = key.mValue.w;
                rotation.x = key.mValue.x;
                rotation.y = key.mValue.y;
                rotation.z = key.mValue.z;
                channel.rotationKeys.emplace_back(key.mTime, rotation);
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

void MeshImporter::ProcessBoneWeights(const ::aiMesh* mesh, Entity* meshEntity, 
                                      const std::map<std::string, int>& boneNameToIndex)
{
    if (!mesh || !meshEntity || !mesh->HasBones())
        return;

    auto& skinComp = meshEntity->AddComponent<FBXSkinComponent>();
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
