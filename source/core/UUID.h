#pragma once

#include <cstdint>
#include <random>
#include <functional>

namespace Core {

    // Stable UUID component that persists across entity recreation
    // This ensures entities can be tracked even when their entt::entity ID changes
    struct UUID {
        uint64_t value;

        UUID() : value(generate()) {}
        explicit UUID(uint64_t val) : value(val) {}

        // Generate a new UUID using random number generation
        static uint64_t generate() {
            static std::random_device rd;
            static std::mt19937_64 gen(rd());
            static std::uniform_int_distribution<uint64_t> dis;
            return dis(gen);
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
