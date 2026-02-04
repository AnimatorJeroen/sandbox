#pragma once
#include <fstream>
#include <vector>
#include <array>
#include <type_traits>

// Include component definitions and Core types first
#include "core/UUID.h"
#include "Components.h"

namespace ComponentSerialization {

    // Helper trait to check if a type is trivially serializable
    template<typename T>
    struct is_trivially_serializable : std::is_trivially_copyable<T> {};

    // Specialize for types that are effectively trivial but have non-trivial constructors
    template<> struct is_trivially_serializable<Core::UUID> : std::true_type {};

    // === Forward declarations (needed for recursion) ===
    template<typename T>
    void Serialize(std::ofstream& file, const T& value);
    
    template<typename T>
    void Deserialize(std::ifstream& file, T& value);

    // === Serialization for std::vector ===
    // This automatically handles ALL vectors of any type T
    template<typename T>
    void Serialize(std::ofstream& file, const std::vector<T>& vec) {
        uint64_t size = vec.size();
        file.write(reinterpret_cast<const char*>(&size), sizeof(size));
        
        if (size > 0) {
            if constexpr (is_trivially_serializable<T>::value) {
                // Fast path: write entire array at once for POD types
                file.write(reinterpret_cast<const char*>(vec.data()), size * sizeof(T));
            } else {
                // Recursive path: serialize each element (handles nested vectors, etc.)
                for (const auto& item : vec) {
                    Serialize(file, item);
                }
            }
        }
    }

    template<typename T>
    void Deserialize(std::ifstream& file, std::vector<T>& vec) {
        uint64_t size = 0;
        file.read(reinterpret_cast<char*>(&size), sizeof(size));
        
        vec.resize(size);
        if (size > 0) {
            if constexpr (is_trivially_serializable<T>::value) {
                // Fast path: read entire array at once for POD types
                file.read(reinterpret_cast<char*>(vec.data()), size * sizeof(T));
            } else {
                // Recursive path: deserialize each element
                for (auto& item : vec) {
                    Deserialize(file, item);
                }
            }
        }
    }

    // === Serialization for std::array ===
    template<typename T, std::size_t N>
    void Serialize(std::ofstream& file, const std::array<T, N>& arr) {
        if constexpr (is_trivially_serializable<T>::value) {
            file.write(reinterpret_cast<const char*>(arr.data()), N * sizeof(T));
        } else {
            for (const auto& item : arr) {
                Serialize(file, item);
            }
        }
    }

    template<typename T, std::size_t N>
    void Deserialize(std::ifstream& file, std::array<T, N>& arr) {
        if constexpr (is_trivially_serializable<T>::value) {
            file.read(reinterpret_cast<char*>(arr.data()), N * sizeof(T));
        } else {
            for (auto& item : arr) {
                Deserialize(file, item);
            }
        }
    }

    // === Default serialization for all other types ===
    // Handles POD types and structs that only contain POD or serializable members
    template<typename T>
    void Serialize(std::ofstream& file, const T& value) {
        // For trivial types, write bytes directly
        if constexpr (is_trivially_serializable<T>::value) {
            file.write(reinterpret_cast<const char*>(&value), sizeof(T));
        } else {
            // For complex types with vectors/arrays, you need to add a specialization below
            static_assert(is_trivially_serializable<T>::value, 
                "Type contains vectors or non-trivial members. Add a template specialization below.");
        }
    }

    template<typename T>
    void Deserialize(std::ifstream& file, T& value) {
        if constexpr (is_trivially_serializable<T>::value) {
            file.read(reinterpret_cast<char*>(&value), sizeof(T));
        }
        // For non-trivial types, specialization below will be called instead
    }

} // namespace ComponentSerialization

// ==================================================================================
// === Component Specializations (for custom serialization) ===
// ==================================================================================
// Add specializations here for components that contain std::vector or other complex types.
// The vector serialization above will handle the actual vector data automatically.
//
// Example template to copy:
//
//    template<>
//    inline void Serialize<YourComponent>(std::ofstream& file, const YourComponent& c) {
//        Serialize(file, c.member1);  // These calls will automatically use
//        Serialize(file, c.member2);  // the appropriate std::vector<T> serializer
//        Serialize(file, c.member3);
//    }
//
//    template<>
//    inline void Deserialize<YourComponent>(std::ifstream& file, YourComponent& c) {
//        Deserialize(file, c.member1);
//        Deserialize(file, c.member2);
//        Deserialize(file, c.member3);
//    }
//
// ==================================================================================

namespace ComponentSerialization {

    // Children: contains std::vector<entt::entity>
    template<>
    inline void Serialize<Children>(std::ofstream& file, const Children& component) {
        Serialize(file, component.children);  // Automatically uses vector<entt::entity> serializer
    }

    template<>
    inline void Deserialize<Children>(std::ifstream& file, Children& component) {
        Deserialize(file, component.children);
    }

    // Parent: contains Core::UUID (marked as trivially serializable)
    template<>
    inline void Serialize<Parent>(std::ofstream& file, const Parent& component) {
        Serialize(file, component.parentUUID);  // UUID is trivially serializable
    }

    template<>
    inline void Deserialize<Parent>(std::ifstream& file, Parent& component) {
        Deserialize(file, component.parentUUID);
    }

    // FBXSkeletonComponent: contains std::vector<FBXBone> + String64
    template<>
    inline void Serialize<FBXSkeletonComponent>(std::ofstream& file, const FBXSkeletonComponent& component) {
        Serialize(file, component.bones);          // vector<FBXBone> handled automatically
        Serialize(file, component.skeletonName);
    }

    template<>
    inline void Deserialize<FBXSkeletonComponent>(std::ifstream& file, FBXSkeletonComponent& component) {
        Deserialize(file, component.bones);
        Deserialize(file, component.skeletonName);
    }

    // FBXSkinComponent: contains std::vector<std::array<FBXVertexWeight, 4>> + int
    template<>
    inline void Serialize<FBXSkinComponent>(std::ofstream& file, const FBXSkinComponent& component) {
        Serialize(file, component.vertexWeights);  // vector<array<T,4>> handled automatically
        Serialize(file, component.skeletonEntityIndex);
    }

    template<>
    inline void Deserialize<FBXSkinComponent>(std::ifstream& file, FBXSkinComponent& component) {
        Deserialize(file, component.vertexWeights);
        Deserialize(file, component.skeletonEntityIndex);
    }

    // FBXAnimationChannel: contains multiple vectors
    template<>
    inline void Serialize<FBXAnimationChannel>(std::ofstream& file, const FBXAnimationChannel& component) {
        Serialize(file, component.boneName);
        Serialize(file, component.positionKeys);   // All vectors handled automatically
        Serialize(file, component.rotationKeys);
        Serialize(file, component.scaleKeys);
    }

    template<>
    inline void Deserialize<FBXAnimationChannel>(std::ifstream& file, FBXAnimationChannel& component) {
        Deserialize(file, component.boneName);
        Deserialize(file, component.positionKeys);
        Deserialize(file, component.rotationKeys);
        Deserialize(file, component.scaleKeys);
    }

    // FBXAnimationClip: contains vector of complex types
    template<>
    inline void Serialize<FBXAnimationClip>(std::ofstream& file, const FBXAnimationClip& component) {
        Serialize(file, component.name);
        Serialize(file, component.duration);
        Serialize(file, component.ticksPerSecond);
        Serialize(file, component.channels);       // vector<FBXAnimationChannel> - recursively serialized
    }

    template<>
    inline void Deserialize<FBXAnimationClip>(std::ifstream& file, FBXAnimationClip& component) {
        Deserialize(file, component.name);
        Deserialize(file, component.duration);
        Deserialize(file, component.ticksPerSecond);
        Deserialize(file, component.channels);
    }

    // FBXAnimationComponent: contains vector of clips
    template<>
    inline void Serialize<FBXAnimationComponent>(std::ofstream& file, const FBXAnimationComponent& component) {
        Serialize(file, component.clips);          // vector<FBXAnimationClip> - recursively serialized
        Serialize(file, component.activeClipIndex);
        Serialize(file, component.currentTime);
        Serialize(file, component.isPlaying);
        Serialize(file, component.loop);
    }

    template<>
    inline void Deserialize<FBXAnimationComponent>(std::ifstream& file, FBXAnimationComponent& component) {
        Deserialize(file, component.clips);
        Deserialize(file, component.activeClipIndex);
        Deserialize(file, component.currentTime);
        Deserialize(file, component.isPlaying);
        Deserialize(file, component.loop);
    }

} // namespace ComponentSerialization
