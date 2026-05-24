#pragma once
#include "../vendor/include/entt/entt.hpp"
#include "core/Registry.h"
#include <unordered_set>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <fstream>
#include <cstdint>

namespace Core {

    // Generic SelectionArchive that can capture any set of component types
    template<class... Cs>
    struct SelectionArchive {
        // Default constructor for empty archive
        SelectionArchive() = default;

        // Constructor with selection set (for capturing from registry)
        explicit SelectionArchive(const std::unordered_set<entt::entity>& set)
            : sel(&set) {
        }

        // Move constructor
        SelectionArchive(SelectionArchive&&) = default;
        SelectionArchive& operator=(SelectionArchive&&) = default;

        // Delete copy (large data)
        SelectionArchive(const SelectionArchive&) = delete;
        SelectionArchive& operator=(const SelectionArchive&) = delete;

        const std::unordered_set<entt::entity>* sel{}; // not owning (nullptr when standalone)

        // Store captured entities (original ids)
        std::vector<entt::entity> entities;

        // Per-component captured payloads
        std::tuple<std::vector<std::pair<entt::entity, Cs>>...> storages;

        // Entities sink for entt::snapshot - accepts underlying type
        void operator()(std::underlying_type_t<entt::entity> e) {
            entt::entity entity{e};
            if (!sel || sel->count(entity)) {
                entities.push_back(entity);
            }
        }
        
        // Entities sink for entt::snapshot - accepts entity type
        void operator()(entt::entity e) {
            if (!sel || sel->count(e)) {
                entities.push_back(e);
            }
        }

        // Generic component capture
        template<class C>
        void capture(entt::entity e, const C& c) {
            if (!sel || sel->count(e)) {
                auto& vec = std::get<std::vector<std::pair<entt::entity, C>>>(storages);
                vec.emplace_back(e, c); // copy-by-value
            }
        }

        // Component sink for entt::snapshot - with underlying entity type
        template<class C>
        void operator()(std::underlying_type_t<entt::entity> e, const C& c) {
            capture<C>(entt::entity{e}, c);
        }

        // Component sink for entt::snapshot - with entity type
        template<class C>
        void operator()(entt::entity e, const C& c) {
            capture<C>(e, c);
        }

        // Binary file serialization - Save to file
        bool SaveToFile(const std::string& filepath) const {
            std::ofstream file(filepath, std::ios::binary);
            if (!file.is_open()) {
                return false;
            }

            // Write number of entities
            uint64_t entityCount = entities.size();
            file.write(reinterpret_cast<const char*>(&entityCount), sizeof(entityCount));

            // Write entity IDs
            file.write(reinterpret_cast<const char*>(entities.data()), 
                      entityCount * sizeof(entt::entity));

            // Write each component storage
            WriteStorages(file, std::index_sequence_for<Cs...>{});

            return file.good();
        }

        // Binary file serialization - Load from file
        bool LoadFromFile(const std::string& filepath) {
            std::ifstream file(filepath, std::ios::binary);
            if (!file.is_open()) {
                return false;
            }

            // Read number of entities
            uint64_t entityCount = 0;
            file.read(reinterpret_cast<char*>(&entityCount), sizeof(entityCount));
            
            // Read entity IDs
            entities.resize(entityCount);
            file.read(reinterpret_cast<char*>(entities.data()), 
                     entityCount * sizeof(entt::entity));

            // Read each component storage
            ReadStorages(file, std::index_sequence_for<Cs...>{});

            return file.good();
        }

    private:
        // Helper to write a single component storage
        template<typename C>
        void WriteComponentStorage(std::ofstream& file, 
                                   const std::vector<std::pair<entt::entity, C>>& storage) const {
            uint64_t count = storage.size();
            file.write(reinterpret_cast<const char*>(&count), sizeof(count));

            for (const auto& [entity, component] : storage) {
                file.write(reinterpret_cast<const char*>(&entity), sizeof(entity));
                file.write(reinterpret_cast<const char*>(&component), sizeof(component));
            }
        }

        // Helper to read a single component storage
        template<typename C>
        void ReadComponentStorage(std::ifstream& file, 
                                 std::vector<std::pair<entt::entity, C>>& storage) {
            uint64_t count = 0;
            file.read(reinterpret_cast<char*>(&count), sizeof(count));

            storage.resize(count);
            for (uint64_t i = 0; i < count; ++i) {
                entt::entity entity;
                C component;
                file.read(reinterpret_cast<char*>(&entity), sizeof(entity));
                file.read(reinterpret_cast<char*>(&component), sizeof(component));
                storage[i] = {entity, component};
            }
        }

        // Write all storages using fold expression
        template<std::size_t... Is>
        void WriteStorages(std::ofstream& file, std::index_sequence<Is...>) const {
            (WriteComponentStorage(file, std::get<Is>(storages)), ...);
        }

        // Read all storages using fold expression
        template<std::size_t... Is>
        void ReadStorages(std::ifstream& file, std::index_sequence<Is...>) {
            (ReadComponentStorage(file, std::get<Is>(storages)), ...);
        }
    };

    namespace ArchiveHelpers
    {
        // Helper to create a snapshot of selected entities with specified components
        template<class... Cs>
        SelectionArchive<Cs...> MakeSnapshot(
            Core::Registry& reg,
            const std::unordered_set<entt::entity>& sel)
        {
            SelectionArchive<Cs...> archive{ sel };

            // Manually iterate and capture entities with their components
            for (auto entity : sel) {
                if (reg.valid(entity)) {
                    archive.entities.push_back(entity);

                    // Capture each component type
                    ((reg.all_of<Cs>(entity) ? archive.template capture<Cs>(entity, reg.get<Cs>(entity)) : void()), ...);
                }
            }

            return archive;
        }

        // Helper: Restore a single component type using the remap table
        template<typename C>
        void restore_component_set(
            Core::Registry& reg,
            const std::vector<std::pair<entt::entity, C>>& components,
            const std::unordered_map<entt::entity, entt::entity>& remap)
        {
            for (const auto& [old_e, component] : components) {
                auto it = remap.find(old_e);
                if (it != remap.end()) {
                    entt::entity new_e = it->second;

                    if constexpr (std::is_same_v<C, Core::UUID>) {
                        reg.emplace_or_replace<C>(new_e, component);
                        reg.ModifyUUID(new_e, component);
                    }
                    else {
                        reg.emplace<C>(new_e, component);
                    }
                }
            }
        }

		// Helper: Restore all component types from archive using fold expression
		template<class... Cs, std::size_t... Is>
		void restore_all_components(
			Core::Registry& reg,
			const SelectionArchive<Cs...>& archive,
			const std::unordered_map<entt::entity, entt::entity>& remap,
			std::index_sequence<Is...>)
		{
			(restore_component_set(reg, std::get<Is>(archive.storages), remap), ...);
		}

		// Unified helper: Restore entities and all their components from a SelectionArchive
		// Creates new entities, builds remap table, and restores all component types
		// Returns the new entities created
		template<class... Cs>
		std::unordered_set<entt::entity> RestoreEntitiesAndComponents(
			Core::Registry& reg,
			const SelectionArchive<Cs...>& archive)
		{
			// Create new entities and build remap table
			std::unordered_map<entt::entity, entt::entity> remap;
			remap.reserve(archive.entities.size());

			for (auto old_e : archive.entities) {
				entt::entity new_e = reg.Create();
				remap[old_e] = new_e;
			}

			restore_all_components(reg, archive, remap, std::index_sequence_for<Cs...>{});

			// Collect the new entities
			std::unordered_set<entt::entity> newEntities;
			for (const auto& [oldEntity, newEntity] : remap) {
				newEntities.insert(newEntity);
			}

			return newEntities;
		}

        // Create a snapshot of ALL entities in registry (for full scene save)
        template<class... Cs>
        SelectionArchive<Cs...> MakeFullSnapshot(Core::Registry& reg)
        {
            SelectionArchive<Cs...> archive;

            // Capture all valid entities
            // Use a view to iterate entities rather than storage directly
            for (auto [entity] : reg.storage<entt::entity>().each()) {
                archive.entities.push_back(entity);

                // Capture each component type if entity has it
                ((reg.all_of<Cs>(entity) ? archive.template capture<Cs>(entity, reg.get<Cs>(entity)) : void()), ...);
            }

            return archive;
        }
    }

} // namespace Core

