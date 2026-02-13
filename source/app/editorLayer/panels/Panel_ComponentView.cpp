#include "pch.h"
#include "Panel_ComponentView.h"
#include "app/editorLayer/EditorContext.h"
#include "app/sceneLayer/Scene.h"
#include "app/sceneLayer/components/Components.h"
#include <imgui/imgui.h>
#include <core/Logger.h>
#include <core/UUID.h>

Panel_ComponentView::Panel_ComponentView(Scene& scene, EditorContext& editorContext)
    : _scene(&scene), _editorContext(editorContext)
{
}

void Panel_ComponentView::SetContext(Scene& scene)
{
    _scene = &scene;
}

void Panel_ComponentView::Render()
{
    if (!_scene)
        return;
    
    // Get the selected entities from editor context
    const auto& selectedEntities = _editorContext.GetSelectedEntities();
    
    ImGui::BeginChild("ComponentView", ImVec2(0, 250), true);
    
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
    
    // Render each component type
    if (entity.HasComponent<NameComponent>())
    {
        RenderNameComponent(entity);
    }
    
    if (entity.HasComponent<Transform>())
    {
        RenderTransformComponent(entity);
    }
    
    if (entity.HasComponent<MeshComponent>())
    {
        RenderMeshComponent(entity);
    }
    
    if (entity.HasComponent<CameraComponent>())
    {
        RenderCameraComponent(entity);
    }
    
    if (entity.HasComponent<Parent>())
    {
        RenderParentComponent(entity);
    }
    
    if (entity.HasComponent<Children>())
    {
        RenderChildrenComponent(entity);
    }
    
    // Render FBX-specific components
    if (entity.HasComponent<FBXSkeletonComponent>() || 
        entity.HasComponent<FBXSkinComponent>() || 
        entity.HasComponent<FBXAnimationComponent>())
    {
        RenderFBXComponents(entity);
    }
}

void Panel_ComponentView::RenderNameComponent(Entity entity)
{
    if (ImGui::CollapsingHeader("Name Component", ImGuiTreeNodeFlags_DefaultOpen))
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
}

void Panel_ComponentView::RenderTransformComponent(Entity entity)
{
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto& transform = entity.GetComponent<Transform>();
        
        ImGui::Text("Position");
        float pos[3] = { transform.Position.x, transform.Position.y, transform.Position.z };
        if (ImGui::DragFloat3("##Position", pos, 0.1f))
        {
            transform.Position = vec3{ pos[0], pos[1], pos[2] };
        }
        
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            _editorContext.BeginUndo();
            _editorContext.applicator().SetField(entity.GetHandle(), "Transform.Position", vec3{ pos[0], pos[1], pos[2] });
            _editorContext.EndUndo();
        }
        
        ImGui::Text("Rotation");
        float rot[3] = { transform.Rotation.x, transform.Rotation.y, transform.Rotation.z };
        if (ImGui::DragFloat3("##Rotation", rot, 0.1f))
        {
            transform.Rotation = vec3{ rot[0], rot[1], rot[2] };
        }
        
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            _editorContext.BeginUndo();
            _editorContext.applicator().SetField(entity.GetHandle(), "Transform.Rotation", vec3{ rot[0], rot[1], rot[2] });
            _editorContext.EndUndo();
        }
        
        ImGui::Text("Scale");
        float scale[3] = { transform.Scale.x, transform.Scale.y, transform.Scale.z };
        if (ImGui::DragFloat3("##Scale", scale, 0.1f))
        {
            transform.Scale = vec3{ scale[0], scale[1], scale[2] };
        }
        
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            _editorContext.BeginUndo();
            _editorContext.applicator().SetField(entity.GetHandle(), "Transform.Scale", vec3{ scale[0], scale[1], scale[2] });
            _editorContext.EndUndo();
        }
    }
}

void Panel_ComponentView::RenderMeshComponent(Entity entity)
{
    if (ImGui::CollapsingHeader("Mesh Component"))
    {
        const auto& mesh = entity.GetComponent<MeshComponent>();
        
        ImGui::Text("Filepath: %s", mesh.filepath.data);
        ImGui::Text("Vertices: %zu", mesh.vertices.size());
        ImGui::Text("Normals: %zu", mesh.normals.size());
        ImGui::Text("TexCoords: %zu", mesh.texCoords.size());
        ImGui::Text("Indices: %zu", mesh.indices.size());
    }
}

void Panel_ComponentView::RenderCameraComponent(Entity entity)
{
    if (ImGui::CollapsingHeader("Camera Component"))
    {
        auto& camera = entity.GetComponent<CameraComponent>();
        
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", camera.position.x, camera.position.y, camera.position.z);
        ImGui::Text("Target: (%.2f, %.2f, %.2f)", camera.target.x, camera.target.y, camera.target.z);
        ImGui::Text("Up: (%.2f, %.2f, %.2f)", camera.up.x, camera.up.y, camera.up.z);
        ImGui::Text("FOV: %.2f", camera.fov);
        ImGui::Text("Near Plane: %.4f", camera.nearPlane);
        ImGui::Text("Far Plane: %.2f", camera.farPlane);
    }
}

void Panel_ComponentView::RenderParentComponent(Entity entity)
{
    if (ImGui::CollapsingHeader("Parent Component"))
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
}

void Panel_ComponentView::RenderChildrenComponent(Entity entity)
{
    if (ImGui::CollapsingHeader("Children Component"))
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
}

void Panel_ComponentView::RenderFBXComponents(Entity entity)
{
    if (entity.HasComponent<FBXSkeletonComponent>())
    {
        if (ImGui::CollapsingHeader("FBX Skeleton Component"))
        {
            const auto& skeleton = entity.GetComponent<FBXSkeletonComponent>();
            ImGui::Text("Bones: %zu", skeleton.bones.size());
        }
    }
    
    if (entity.HasComponent<FBXSkinComponent>())
    {
        if (ImGui::CollapsingHeader("FBX Skin Component"))
        {
            const auto& skin = entity.GetComponent<FBXSkinComponent>();
            ImGui::Text("Skeleton Entity Index: %d", skin.skeletonEntityIndex);
            ImGui::Text("Vertex Weights: %zu", skin.vertexWeights.size());
        }
    }
    
    if (entity.HasComponent<FBXAnimationComponent>())
    {
        if (ImGui::CollapsingHeader("FBX Animation Component"))
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
    }
}
