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

    // === Trait to detect if a type likely contains vectors ===
    // This checks if the type is NOT trivially copyable (vectors aren't)
    // but also not a known container type itself
    template<typename T, typename = void>
    struct potentially_has_vectors : std::false_type {};
    
    template<typename T>
    struct potentially_has_vectors<T, typename std::enable_if<
        !std::is_trivially_copyable<T>::value && 
        !is_trivially_serializable<T>::value
    >::type> : std::true_type {};
    
    // Don't flag vectors/arrays themselves as needing specialization
    template<typename T>
    struct potentially_has_vectors<std::vector<T>> : std::false_type {};
    
    template<typename T, std::size_t N>
    struct potentially_has_vectors<std::array<T, N>> : std::false_type {};

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
    // Handles POD types and catches types that need specialization
    template<typename T>
    void Serialize(std::ofstream& file, const T& value) {
        // For trivial types, write bytes directly
        if constexpr (is_trivially_serializable<T>::value) {
            file.write(reinterpret_cast<const char*>(&value), sizeof(T));
        } else {
            // This will trigger an error showing the type name in the template context
            static_assert(sizeof(T) == 0,
                "\n\n"
                "========================================\n"
                "SERIALIZATION ERROR: Missing template specialization!\n"
                "========================================\n"
                "Look for the type 'T=' in the error message above.\n"
                "That type contains std::vector or other complex members\n"
                "and needs a specialization in ComponentSerialization.h\n\n"
                "Add this code:\n\n"
                "  template<>\n"
                "  inline void Serialize<YourType>(std::ofstream& file, const YourType& c) {\n"
                "      Serialize(file, c.member1);  // List ALL members in order\n"
                "      Serialize(file, c.member2);\n"
                "  }\n\n"
                "  template<>\n"
                "  inline void Deserialize<YourType>(std::ofstream& file, YourType& c) {\n"
                "      Deserialize(file, c.member1);\n"
                "      Deserialize(file, c.member2);\n"
                "  }\n"
                "========================================\n");
        }
    }

    template<typename T>
    void Deserialize(std::ifstream& file, T& value) {
        if constexpr (is_trivially_serializable<T>::value) {
            file.read(reinterpret_cast<char*>(&value), sizeof(T));
        } else {
            static_assert(sizeof(T) == 0,
                "\n\n"
                "========================================\n"
                "DESERIALIZATION ERROR: Missing template specialization!\n"
                "========================================\n"
                "Look for 'T=' in the error above to see which type needs a specialization.\n"
                "Add both Serialize<T> and Deserialize<T> specializations.\n"
                "========================================\n");
        }
    }

} // namespace ComponentSerialization

// ==================================================================================
// === Component Specializations (REQUIRED for types with std::vector or complex members) ===
// ==================================================================================
// If you get a compile error here, it means you're trying to serialize a type that
// contains vectors or other non-trivial members without providing a specialization.
//
// The error message will tell you which type needs a specialization.
// Add it below using this template:
//
//    template<>
//    inline void Serialize<YourComponent>(std::ofstream& file, const YourComponent& c) {
//        Serialize(file, c.member1);  // Vectors are handled automatically
//        Serialize(file, c.member2);  // Just list all members in order
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

    // Parent: contains Core::UUID (marked as trivially serializable, but needs explicit handling)
    template<>
    inline void Serialize<Parent>(std::ofstream& file, const Parent& component) {
        Serialize(file, component.parentUUID);
    }

    template<>
    inline void Deserialize<Parent>(std::ifstream& file, Parent& component) {
        Deserialize(file, component.parentUUID);
    }

    // FBXSkeletonComponent: contains std::vector<FBXBone> + String64
    template<>
    inline void Serialize<FBXSkeletonComponent>(std::ofstream& file, const FBXSkeletonComponent& component) {
        Serialize(file, component.skeletonName);
    }

    template<>
    inline void Deserialize<FBXSkeletonComponent>(std::ifstream& file, FBXSkeletonComponent& component) {
        Deserialize(file, component.skeletonName);
    }

    // FBXBone: contains String64, int, std::vector<int>, and mat4 members
    template<>
    inline void Serialize<FBXBone>(std::ofstream& file, const FBXBone& component) {
        Serialize(file, component.name);
        Serialize(file, component.offsetMatrix);
        Serialize(file, component.localRestTransform);
    }

    template<>
    inline void Deserialize<FBXBone>(std::ifstream& file, FBXBone& component) {
        Deserialize(file, component.name);
        Deserialize(file, component.offsetMatrix);
        Deserialize(file, component.localRestTransform);
    }

    // FBXSkinComponent: contains std::vector<std::array<FBXVertexWeight, 4>> + int
    template<>
    inline void Serialize<FBXSkinComponent>(std::ofstream& file, const FBXSkinComponent& component) {
        Serialize(file, component.vertexWeights);
        Serialize(file, component.skeletonEntityIndex);
    }

    template<>
    inline void Deserialize<FBXSkinComponent>(std::ifstream& file, FBXSkinComponent& component) {
        Deserialize(file, component.vertexWeights);
        Deserialize(file, component.skeletonEntityIndex);
    }

	// FBXAnimationChannels: contains std::vector<FBXAnimationChannel>
    template<>
    inline void Serialize<FBXAnimationChannels>(std::ofstream& file, const FBXAnimationChannels& component) {
       Serialize(file, component.channels);
    }

	template<>
    inline void Deserialize<FBXAnimationChannels>(std::ifstream& file, FBXAnimationChannels& component) {
        Deserialize(file, component.channels);
    }

    // FBXAnimationChannel: contains multiple vectors
    template<>
    inline void Serialize<FBXAnimationChannel>(std::ofstream& file, const FBXAnimationChannel& component) {
		Serialize(file, component.clipIndex);
        Serialize(file, component.positionKeys);
        Serialize(file, component.rotationKeys);
        Serialize(file, component.scaleKeys);
    }

    template<>
    inline void Deserialize<FBXAnimationChannel>(std::ifstream& file, FBXAnimationChannel& component) {
        Deserialize(file, component.clipIndex);
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
    }

    template<>
    inline void Deserialize<FBXAnimationClip>(std::ifstream& file, FBXAnimationClip& component) {
        Deserialize(file, component.name);
        Deserialize(file, component.duration);
        Deserialize(file, component.ticksPerSecond);
    }

    // FBXAnimationComponent: contains vector of clips
    template<>
    inline void Serialize<FBXAnimationComponent>(std::ofstream& file, const FBXAnimationComponent& component) {
        Serialize(file, component.clips);
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

	// MeshComponent: contains std::vector<uint32_t>
    template<>
    inline void Serialize<MeshComponent>(std::ofstream& file, const MeshComponent& component) {
        Serialize(file, component.filepath);
        Serialize(file, component.indices);
        Serialize(file, component.normals);
        Serialize(file, component.vertices);
        Serialize(file, component.texCoords);
    }
    template<>
    inline void Deserialize<MeshComponent>(std::ifstream& file, MeshComponent& component) {
        Deserialize(file, component.filepath);
        Deserialize(file, component.indices);
        Deserialize(file, component.normals);
        Deserialize(file, component.vertices);
        Deserialize(file, component.texCoords);
	}

    //runtime components, don't need to serialize
    template<>
    inline void Serialize<Children>(std::ofstream& file, const Children& component) {}
} // namespace ComponentSerialization
