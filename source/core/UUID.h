#pragma once

#include <cstdint>
#include <random>
#include <functional>
#include <chrono>

namespace Core {

    // Stable UUID component that persists across entity recreation
    // This ensures entities can be tracked even when their entt::entity ID changes
    // UUIDs are ordered by creation time - earlier created entities have lower UUID values
    struct UUID {
        uint64_t value;

        UUID() : value(generate()) {}
        explicit UUID(uint64_t val) : value(val) {}
        
        // Explicit copy constructor - preserves the UUID value
        UUID(const UUID& other) : value(other.value) {}
        
        // Explicit copy assignment - preserves the UUID value
        UUID& operator=(const UUID& other) {
            value = other.value;
            return *this;
        }
        
        // Move constructor - preserves the UUID value
        UUID(UUID&& other) noexcept : value(other.value) {}
        
        // Move assignment - preserves the UUID value
        UUID& operator=(UUID&& other) noexcept {
            value = other.value;
            return *this;
        }

        // Generate a new UUID using system time + random number
        // Format: [48 bits: milliseconds since epoch][16 bits: random]
        // This ensures UUIDs are ordered chronologically by creation time
        static uint64_t generate() {
            // Get current time in milliseconds since epoch
            auto now = std::chrono::system_clock::now();
            auto duration = now.time_since_epoch();
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            
            // Use 48 bits for timestamp (good for ~8900 years from 1970)
            uint64_t timestamp = static_cast<uint64_t>(millis) & 0xFFFFFFFFFFFFULL;
            
            // Generate 16 bits of randomness for uniqueness within same millisecond
            static std::random_device rd;
            static std::mt19937_64 gen(rd());
            static std::uniform_int_distribution<uint64_t> dis(0, 0xFFFF);
            uint64_t random_part = dis(gen);
            
            // Combine: [48-bit timestamp][16-bit random]
            return (timestamp << 16) | random_part;
        }

        // Comparison operators for use in maps/sets
        bool operator==(const UUID& other) const { return value == other.value; }
        bool operator!=(const UUID& other) const { return value != other.value; }
        bool operator<(const UUID& other) const { return value < other.value; }

        // Serialization support
        template<class Archive>
        void serialize(Archive& ar) {
            ar(value);
        }

        const static UUID Null;

    };

} // namespace Core

// Hash specialization for use in unordered containers
namespace std {
    template<>
    struct hash<Core::UUID> {
        size_t operator()(const Core::UUID& uuid) const {
            return std::hash<uint64_t>{}(uuid.value);
        }
    };
}
