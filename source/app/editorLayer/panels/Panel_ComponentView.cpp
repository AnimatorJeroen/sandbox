#include "pch.h"
#include "Panel_ComponentView.h"
#include "app/editorLayer/EditorContext.h"
#include "app/sceneLayer/Scene.h"
#include "app/sceneLayer/components/Components.h"
#include <imgui/imgui.h>
#include <core/Logger.h>
#include <core/UUID.h>
#include <functional>

Panel_ComponentView::Panel_ComponentView(Scene& scene, EditorContext& editorContext)
    : _scene(&scene), _editorContext(editorContext)
{
}

void Panel_ComponentView::SetContext(Scene& scene)
{
    _scene = &scene;
}

void Panel_ComponentView::Render(float height)
{
    if (!_scene)
        return;
    
    // Get the selected entities from editor context
    const auto& selectedEntities = _editorContext.GetSelectedEntities();
    
    // Use provided height, or 0 for auto-sizing (will use remaining space)
    ImGui::BeginChild("ComponentView", ImVec2(0, height), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    
    if (selectedEntities.empty())
    {
        ImGui::TextDisabled("No entity selected");
    }
    else if (selectedEntities.size() == 1)
    {
        Entity selectedEntity = *selectedEntities.begin();
        RenderComponentUI(selectedEntity);
    }
    else
    {
        ImGui::TextDisabled("Multiple entities selected (%zu)", selectedEntities.size());
    }
    
    ImGui::EndChild();
}

void Panel_ComponentView::RenderComponentUI(Entity entity)
{
    if (!entity)
    {
        ImGui::TextDisabled("Invalid entity");
        return;
    }
    
    // Display entity header with name and UUID
    const NameComponent* nameComp = entity.HasComponent<NameComponent>() 
        ? &entity.GetComponent<NameComponent>() 
        : nullptr;
    
    ImGui::Text("Entity: %s", nameComp ? nameComp->name.data : "Unnamed");
    ImGui::SameLine();
    ImGui::TextDisabled("(UUID: %llu)", entity.UUID().value);
    ImGui::Separator();
    

    RenderComponent<NameComponent>(entity, [this](Entity e) {
        RenderNameComponent(e);
        });
    RenderComponent<Transform>(entity, [this](Entity e) {
        RenderTransformComponent(e);
        });
    RenderComponent<MeshComponent>(entity, [this](Entity e) {
        RenderMeshComponent(e);
        });
    RenderComponent<CameraComponent>(entity, [this](Entity e) {
        RenderCameraComponent(e);
        });
    RenderComponent<Parent>(entity, [this](Entity e) {
        RenderParentComponent(e);
        });
    RenderComponent<FBXSkeletonComponent>(entity, [this](Entity e) {
        RenderFBXSkeletonComponent(e);
        });
    RenderComponent<FBXSkinComponent>(entity, [this](Entity e) {
        RenderFBXSkinComponent(e);
        });
    RenderComponent<FBXAnimationComponent>(entity, [this](Entity e) {
        RenderFBXAnimationComponent(e);
        });
    RenderComponent<FBXBone>(entity, [this](Entity e) {
        RenderFBXBoneComponent(e);
        });
    RenderComponent<FBXAnimationChannels>(entity, [this](Entity e) {
        RenderFBXAnimationChannelsComponent(e);
        });

    ImGui::TextDisabled("Runtime Components");
    ImGui::Separator();
    RenderComponent<Children>(entity, [this](Entity e) {
        RenderChildrenComponent(e);
        });

    // Add Component button at the bottom
    ImGui::Separator();
    RenderAddComponentButton(entity);
}

void Panel_ComponentView::RenderNameComponent(Entity entity)
{
    auto& name = entity.GetComponent<NameComponent>();

    char buffer[64];
    strncpy_s(buffer, name.name.data, sizeof(buffer) - 1);

    if (ImGui::InputText("Name", buffer, sizeof(buffer)))
    {
        // Update the name immediately
        name.name = String64(buffer);
    }

    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        _editorContext.BeginUndo();
        _editorContext.applicator().SetField(entity.GetHandle(), "NameComponent.name", String64(buffer));
        _editorContext.EndUndo();
    }
}

void Panel_ComponentView::RenderTransformComponent(Entity entity)
{
    auto& transform = entity.GetComponent<Transform>();
    
    ImGui::Text("Position");
    float pos[3] = { transform.Position.x, transform.Position.y, transform.Position.z };
    if (ImGui::DragFloat3("##Position", pos, 0.1f))
        transform.Position = vec3{ pos[0], pos[1], pos[2] };

    static vec3 posStartVal;
    if (ImGui::IsItemActivated())
        posStartVal = transform.Position;
    
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        _editorContext.BeginUndo();
        transform.Position = posStartVal;
        _editorContext.applicator().SetField(entity.GetHandle(), "Transform.Position", vec3{ pos[0], pos[1], pos[2] });
        _editorContext.EndUndo();
    }
    
    ImGui::Text("Rotation");
    float rot[3] = { transform.Rotation.x, transform.Rotation.y, transform.Rotation.z };
    if (ImGui::DragFloat3("##Rotation", rot, 0.1f))
        transform.Rotation = vec3{ rot[0], rot[1], rot[2] };

    static vec3 rotStartVal;
    if (ImGui::IsItemActivated())
        rotStartVal = transform.Rotation;

    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        _editorContext.BeginUndo();
        transform.Rotation = rotStartVal;
        _editorContext.applicator().SetField(entity.GetHandle(), "Transform.Rotation", vec3{ rot[0], rot[1], rot[2] });
        _editorContext.EndUndo();
    }
    
    ImGui::Text("Scale");
    float scale[3] = { transform.Scale.x, transform.Scale.y, transform.Scale.z };
    if (ImGui::DragFloat3("##Scale", scale, 0.1f))
        transform.Scale = vec3{ scale[0], scale[1], scale[2] };

    static vec3 scaleStartVal;
    if (ImGui::IsItemActivated())
        scaleStartVal = transform.Scale;
    
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        _editorContext.BeginUndo();
        transform.Scale = scaleStartVal;
        _editorContext.applicator().SetField(entity.GetHandle(), "Transform.Scale", vec3{ scale[0], scale[1], scale[2] });
        _editorContext.EndUndo();
    }
}

void Panel_ComponentView::RenderMeshComponent(Entity entity)
{
    const auto& mesh = entity.GetComponent<MeshComponent>();
    
    ImGui::Text("Filepath: %s", mesh.filepath.data);
    ImGui::Text("Vertices: %zu", mesh.vertices.size());
    ImGui::Text("Normals: %zu", mesh.normals.size());
    ImGui::Text("TexCoords: %zu", mesh.texCoords.size());
    ImGui::Text("Indices: %zu", mesh.indices.size());
}

void Panel_ComponentView::RenderCameraComponent(Entity entity)
{
    auto& camera = entity.GetComponent<CameraComponent>();
    
    ImGui::Text("Position: (%.2f, %.2f, %.2f)", camera.position.x, camera.position.y, camera.position.z);
    ImGui::Text("Target: (%.2f, %.2f, %.2f)", camera.target.x, camera.target.y, camera.target.z);
    ImGui::Text("Up: (%.2f, %.2f, %.2f)", camera.up.x, camera.up.y, camera.up.z);
    ImGui::Text("FOV: %.2f", camera.fov);
    ImGui::Text("Near Plane: %.4f", camera.nearPlane);
    ImGui::Text("Far Plane: %.2f", camera.farPlane);
}

void Panel_ComponentView::RenderParentComponent(Entity entity)
{
    const auto& parent = entity.GetComponent<Parent>();
    
    if (parent.HasParent())
    {
        ImGui::Text("Parent UUID: %llu", parent.parentUUID.value);
        
        // Try to find and display parent entity name
        auto view = _scene->GetRegistry().view<Core::UUID>();
        for (auto e : view)
        {
            if (_scene->GetRegistry().get<Core::UUID>(e).value == parent.parentUUID.value)
            {
                Entity parentEntity(e, &_scene->GetRegistry());
                if (parentEntity.HasComponent<NameComponent>())
                {
                    ImGui::SameLine();
                    ImGui::Text("(%s)", parentEntity.GetComponent<NameComponent>().name.data);
                }
                break;
            }
        }
    }
    else
    {
        ImGui::TextDisabled("No parent (root entity)");
    }
}

void Panel_ComponentView::RenderChildrenComponent(Entity entity)
{
    const auto& children = entity.GetComponent<Children>();
    
    ImGui::Text("Child Count: %zu", children.Count());
    
    if (children.HasChildren())
    {
        ImGui::Indent();
        for (const auto& childHandle : children.children)
        {
            Entity childEntity(childHandle, &_scene->GetRegistry());
            if (childEntity)
            {
                const NameComponent* childName = childEntity.HasComponent<NameComponent>() 
                    ? &childEntity.GetComponent<NameComponent>() 
                    : nullptr;
                
                ImGui::BulletText("%s (UUID: %llu)", 
                    childName ? childName->name.data : "Unnamed",
                    childEntity.UUID().value);
            }
        }
        ImGui::Unindent();
    }
}

void Panel_ComponentView::RenderFBXSkeletonComponent(Entity entity)
{
    const auto& skeleton = entity.GetComponent<FBXSkeletonComponent>();
    ImGui::Text("Bones: %zu", skeleton.bones.size());
}

void Panel_ComponentView::RenderFBXSkinComponent(Entity entity)
{
    const auto& skin = entity.GetComponent<FBXSkinComponent>();
    ImGui::Text("Skeleton Entity Index: %d", skin.skeletonEntityIndex);
    ImGui::Text("Vertex Weights: %zu", skin.vertexWeights.size());
}

void Panel_ComponentView::RenderFBXAnimationComponent(Entity entity)
{
    auto& anim = entity.GetComponent<FBXAnimationComponent>();
    
    ImGui::Text("Clips: %zu", anim.clips.size());
    
    // Active Clip dropdown
    if (!anim.clips.empty())
    {
        // Build list of clip names for the dropdown
        std::vector<const char*> clipNames;
        for (const auto& clip : anim.clips)
        {
            clipNames.push_back(clip.name.data);
        }
        
        // Clamp active clip index to valid range
        int currentClip = anim.activeClipIndex;
        if (currentClip < 0 || currentClip >= static_cast<int>(anim.clips.size()))
        {
            currentClip = 0;
            anim.activeClipIndex = 0;
        }
        
        if (ImGui::Combo("Active Clip", &currentClip, clipNames.data(), static_cast<int>(clipNames.size())))
        {
            anim.activeClipIndex = currentClip;
            anim.currentTime = 0.0; // Reset to start of new clip
            
            _editorContext.BeginUndo();
            _editorContext.applicator().SetField(entity.GetHandle(), "FBXAnimationComponent.activeClipIndex", currentClip);
            _editorContext.applicator().SetField(entity.GetHandle(), "FBXAnimationComponent.currentTime", 0.0);
            _editorContext.EndUndo();
        }
    }
    else
    {
        ImGui::TextDisabled("Active Clip: None");
    }
    
    // Playing checkbox
    bool isPlaying = anim.isPlaying;
    if (ImGui::Checkbox("Playing", &isPlaying))
    {
        anim.isPlaying = isPlaying;
        
        _editorContext.BeginUndo();
        _editorContext.applicator().SetField(entity.GetHandle(), "FBXAnimationComponent.isPlaying", isPlaying);
        _editorContext.EndUndo();
    }
    
    // Loop checkbox
    bool loop = anim.loop;
    if (ImGui::Checkbox("Loop", &loop))
    {
        anim.loop = loop;
        
        _editorContext.BeginUndo();
        _editorContext.applicator().SetField(entity.GetHandle(), "FBXAnimationComponent.loop", loop);
        _editorContext.EndUndo();
    }
    
    // Current Time display (read-only for now)
    ImGui::Text("Current Time: %.2f", anim.currentTime);
}

void Panel_ComponentView::RenderFBXBoneComponent(Entity entity)
{
    const auto& bone = entity.GetComponent<FBXBone>();
    
    // Display offset matrix (inverse bind pose)
    if (ImGui::TreeNode("Offset Matrix (Inverse Bind Pose)"))
    {
        ImGui::TextDisabled("Transforms from mesh space to bone space");
        ImGui::Separator();
        
        for (int i = 0; i < 4; i++)
        {
            ImGui::Text("[%.3f, %.3f, %.3f, %.3f]", 
                bone.offsetMatrix[i][0], 
                bone.offsetMatrix[i][1], 
                bone.offsetMatrix[i][2], 
                bone.offsetMatrix[i][3]);
        }
        
        ImGui::TreePop();
    }
    
    // Display local rest transform
    if (ImGui::TreeNode("Local Rest Transform"))
    {
        ImGui::TextDisabled("Local transform in rest pose");
        ImGui::Separator();
        
        for (int i = 0; i < 4; i++)
        {
            ImGui::Text("[%.3f, %.3f, %.3f, %.3f]", 
                bone.localRestTransform[i][0], 
                bone.localRestTransform[i][1], 
                bone.localRestTransform[i][2], 
                bone.localRestTransform[i][3]);
        }
        
        ImGui::TreePop();
    }
}

void Panel_ComponentView::RenderFBXAnimationChannelsComponent(Entity entity)
{
    const auto& animChannels = entity.GetComponent<FBXAnimationChannels>();
    
    ImGui::Text("Total Channels: %zu", animChannels.channels.size());
    ImGui::Separator();
    
    if (animChannels.channels.empty())
    {
        ImGui::TextDisabled("No animation channels");
    }
    else
    {
        // Display each animation channel
        for (size_t i = 0; i < animChannels.channels.size(); i++)
        {
            const auto& channel = animChannels.channels[i];
            
            char label[64];
            snprintf(label, sizeof(label), "Channel %zu (Clip Index: %d)", i, channel.clipIndex);
            
            if (ImGui::TreeNode(label))
            {
                ImGui::Indent();
                
                // Position keys
                if (ImGui::TreeNode("Position Keys"))
                {
                    ImGui::Text("Count: %zu", channel.positionKeys.size());
                    
                    if (!channel.positionKeys.empty())
                    {
                        ImGui::Separator();
                        for (size_t j = 0; j < std::min(channel.positionKeys.size(), size_t(5)); j++)
                        {
                            const auto& key = channel.positionKeys[j];
                            ImGui::Text("[%.2f] (%.3f, %.3f, %.3f)", 
                                key.time, key.value.x, key.value.y, key.value.z);
                        }
                        
                        if (channel.positionKeys.size() > 5)
                        {
                            ImGui::TextDisabled("... and %zu more", channel.positionKeys.size() - 5);
                        }
                    }
                    
                    ImGui::TreePop();
                }
                
                // Rotation keys
                if (ImGui::TreeNode("Rotation Keys"))
                {
                    ImGui::Text("Count: %zu", channel.rotationKeys.size());
                    
                    if (!channel.rotationKeys.empty())
                    {
                        ImGui::Separator();
                        for (size_t j = 0; j < std::min(channel.rotationKeys.size(), size_t(5)); j++)
                        {
                            const auto& key = channel.rotationKeys[j];
                            ImGui::Text("[%.2f] (%.3f, %.3f, %.3f, %.3f)", 
                                key.time, key.value.x, key.value.y, key.value.z, key.value.w);
                        }
                        
                        if (channel.rotationKeys.size() > 5)
                        {
                            ImGui::TextDisabled("... and %zu more", channel.rotationKeys.size() - 5);
                        }
                    }
                    
                    ImGui::TreePop();
                }
                
                // Scale keys
                if (ImGui::TreeNode("Scale Keys"))
                {
                    ImGui::Text("Count: %zu", channel.scaleKeys.size());
                    
                    if (!channel.scaleKeys.empty())
                    {
                        ImGui::Separator();
                        for (size_t j = 0; j < std::min(channel.scaleKeys.size(), size_t(5)); j++)
                        {
                            const auto& key = channel.scaleKeys[j];
                            ImGui::Text("[%.2f] (%.3f, %.3f, %.3f)", 
                                key.time, key.value.x, key.value.y, key.value.z);
                        }
                        
                        if (channel.scaleKeys.size() > 5)
                        {
                            ImGui::TextDisabled("... and %zu more", channel.scaleKeys.size() - 5);
                        }
                    }
                    
                    ImGui::TreePop();
                }
                
                ImGui::Unindent();
                ImGui::TreePop();
            }
        }
    }
}

void Panel_ComponentView::RenderAddComponentButton(Entity entity)
{
    if (!entity)
        return;
    
    // Center the button
    float buttonWidth = 150.0f;
    float availWidth = ImGui::GetContentRegionAvail().x;
    ImGui::SetCursorPosX((availWidth - buttonWidth) * 0.5f);
    
    if (ImGui::Button("Add Component", ImVec2(buttonWidth, 0)))
    {
        ImGui::OpenPopup("AddComponentPopup");
    }
    
    // Render the popup context menu
    if (ImGui::BeginPopup("AddComponentPopup"))
    {
        ImGui::TextDisabled("Select a component to add:");
        ImGui::Separator();
        
        // Basic Components
        AddComponentMenuItem<Transform>(entity, "Transform");
        AddComponentMenuItem<MeshComponent>(entity, "Mesh Component");

        ImGui::Separator();
        ImGui::TextDisabled("FBX Components:");
        
        // FBX Components
        AddComponentMenuItem<FBXSkeletonComponent>(entity, "FBX Skeleton Component");
        AddComponentMenuItem<FBXSkinComponent>(entity, "FBX Skin Component");
        AddComponentMenuItem<FBXAnimationComponent>(entity, "FBX Animation Component");
        AddComponentMenuItem<FBXBone>(entity, "FBX Bone Component");
        AddComponentMenuItem<FBXAnimationChannels>(entity, "FBX Animation Channels");

        ImGui::EndPopup();
    }
}
