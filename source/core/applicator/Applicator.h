#pragma once
#include "UndoManager.h"
#include "SelectionArchive.h"
#include "Clipboard.h"
#include <memory>
#include <optional>

namespace Core {

    // C++20 compile-time string wrapper for non-type template parameters
    template<std::size_t N>
    struct CompileTimeString {
        constexpr CompileTimeString(const char (&str)[N]) {
            std::copy_n(str, N, value);
        }
        char value[N];
        static constexpr std::size_t size = N;
    };




    template<typename ValueTypes, typename ComponentTypes>
    class Applicator {
    public:
        explicit Applicator(UndoManager<ValueTypes>& undoManager) 
            : _undoManager(undoManager), _clipboard() {}

        //// SetField with compile-time string literal (C++20)
        //// Usage: applicator.SetField<"Transform.Position.x">(entity, value);
        //template<CompileTimeString PathStr, typename T>
        //void SetField(entt::entity e, T&& newVal) {
        //    constexpr auto fullPath = PATH_FULL_CONSTEXPR(PathStr.value);
        //    entt::id_type actualComponentTypeId = resolve_component_type(fullPath.componentType);
        //    _undoManager.SetField(e, actualComponentTypeId, fullPath.propertyPath, ValueTypes{ std::forward<T>(newVal) });
        //}

        // SetField with runtime string (for variables)
        template<typename T>
        void SetField(entt::entity e, const std::string& path_str, T&& newVal) {
            auto fullPath = reflection::ParseFullPathRuntime(path_str.c_str());
            entt::id_type actualComponentTypeId = resolve_component_type(fullPath.componentType);
            _undoManager.SetField(e, actualComponentTypeId, fullPath.propertyPath, ValueTypes{ std::forward<T>(newVal) });
        }

		//captures specified component types
        template<typename... Cs>
        void CaptureCreate(const std::unordered_set<entt::entity>& selection)
        {
            _undoManager.template CaptureCreate<Cs...>(selection);
        }

		// captures all component types in ComponentTypes tuple

        void CaptureCreate(const std::unordered_set<entt::entity>& selection)
        {
			//impl with tuple unpacking of all ComponentTypes
            CaptureCreateImpl(selection, ComponentTypes{});
        }

        //captures specified component types
        template<typename... Cs>
        void CaptureDelete(const std::unordered_set<entt::entity>& selection)
        {
            _undoManager.template CaptureDelete<Cs...>(selection);
        }

        // captures all component types in ComponentTypes tuple
        template<typename SelectionContainer>
        void CaptureDelete(const SelectionContainer& selection)
		{
            std::unordered_set<entt::entity> selectionSet(selection.begin(), selection.end());
            //impl with tuple unpacking of all ComponentTypes
            CaptureDeleteImpl(selectionSet, ComponentTypes{});
		}

        // Capture component add/remove - captures specified component types
        // Note we should always capture UUID as well for entity tracking
        template<typename... Cs>
        void CaptureComponentChange(
            const std::unordered_set<entt::entity>& entities,
            SelectionArchive<Cs...>&& beforeState,
            SelectionArchive<Cs...>&& afterState)
        {
            _undoManager.template CaptureComponentChange<Cs...>(entities, std::move(beforeState), std::move(afterState));
        }

        // Copy selection to clipboard
        template<typename SelectionContainer>
        void CopyToClipboard(const SelectionContainer& selection) {
            // Convert selection to unordered_set
            std::unordered_set<entt::entity> selectionSet(selection.begin(), selection.end());

            // Call the implementation with ComponentTypes tuple
            CopyToClipboardImpl(selectionSet, ComponentTypes{});
        }

        // Paste from clipboard
        void PasteFromClipboard(ClipboardType type) {
            if (type == ClipboardType::Entities) {
                // Call the implementation with ComponentTypes tuple
                PasteFromClipboardImpl(ComponentTypes{});
            }
        }

        // Begin recording patches for bundling into a single undo step
        void BeginUndo() {
            _undoManager.BeginUndo();
        }

        // End recording and push all bundled patches as a single undo step
        void EndUndo() {
            _undoManager.EndUndo();
        }

        // Check if currently recording a bundle
        [[nodiscard]] bool IsRecording() const noexcept {
            return _undoManager.IsRecording();
        }

        // Get the clipboard (for future paste operations)
        [[nodiscard]] const Clipboard<ComponentTypes>& GetClipboard() const noexcept {
            return _clipboard;
        }

    private:
        UndoManager<ValueTypes>& _undoManager;
        Clipboard<ComponentTypes> _clipboard;

        // Helpers to unpack ComponentTypes tuple into variadic template parameters
        template<typename... Cs>
        void CaptureCreateImpl(const std::unordered_set<entt::entity>& selection, std::tuple<Cs...>) {
            _undoManager.template CaptureCreate<Cs...>(selection);
        }

        template<typename... Cs>
        void CaptureDeleteImpl(const std::unordered_set<entt::entity>& selection, std::tuple<Cs...>) {
            _undoManager.template CaptureDelete<Cs...>(selection);
        }

        // Helper for CopyToClipboard with tuple unpacking
        template<typename... Cs>
        void CopyToClipboardImpl(const std::unordered_set<entt::entity>& selection, std::tuple<Cs...>) {
            auto* registry = _undoManager.GetRegistry();
            if (!registry) {
                throw std::runtime_error("Registry not set in UndoManager");
            }

            // Create a snapshot of the selection with all component types
            auto archive = ArchiveHelpers::MakeSnapshot<Cs...>(*registry, selection);

            // Store in clipboard
            _clipboard.StoreEntities(std::move(archive));
        }

        // Helper for PasteFromClipboard with tuple unpacking
        template<typename... Cs>
        void PasteFromClipboardImpl(std::tuple<Cs...>) {
            // Check if clipboard has entities
            if (!_clipboard.HasEntities()) {
                return; // Nothing to paste
            }

            auto* registry = _undoManager.GetRegistry();
            if (!registry) {
                throw std::runtime_error("Registry not set in UndoManager");
            }

            // Get the archive from clipboard (remains unchanged)
            const auto* archive = _clipboard.template GetEntitiesArchive<Cs...>();
            if (!archive || archive->entities.empty()) {
                return; // Nothing to paste
            }

            // Restore entities from clipboard archive (this creates entities with original UUIDs)
            auto newEntities = ArchiveHelpers::RestoreEntitiesAndComponents(*registry, *archive);

            // Build UUID remap table: old UUID -> new UUID
            // Each paste generates fresh UUIDs for all pasted entities
            std::unordered_map<uint64_t, uint64_t> uuidRemap;
            CollectUUIDsFromEntities(newEntities, *registry, uuidRemap);

            // Remap any UUID fields in components on the newly created entities
            RemapUUIDsInEntities<Cs...>(newEntities, *registry, uuidRemap);

            // Capture the creation for undo/redo
            CaptureCreate(newEntities);
        }

        // Collect all UUIDs from newly created entities and generate new UUIDs for them
        void CollectUUIDsFromEntities(
            const std::unordered_set<entt::entity>& entities,
            Core::Registry& registry,
            std::unordered_map<uint64_t, uint64_t>& uuidRemap)
        {
            for (auto entity : entities) {
                if (registry.all_of<UUID>(entity)) {
                    // Get the old UUID
                    uint64_t oldUUID = registry.get<UUID>(entity).value;
                    
                    // Generate a new UUID
                    UUID newUUID;
                    
                    // Store the mapping
                    uuidRemap[oldUUID] = newUUID.value;
                    
                    // Update the UUID component on the entity
                    registry.ModifyUUID(entity, newUUID);
                }
            }
        }

        // Remap any UUID fields in all components on the entities
        // Uses reflection to find UUID-typed fields
        template<typename... Cs>
        void RemapUUIDsInEntities(
            const std::unordered_set<entt::entity>& entities,
            Core::Registry& registry,
            const std::unordered_map<uint64_t, uint64_t>& uuidRemap)
        {
            // Process each component type
            (RemapUUIDsInComponentsOnEntities<Cs>(entities, registry, uuidRemap), ...);
        }

        // Helper to remap UUIDs in a specific component type on the entities
        template<typename C>
        void RemapUUIDsInComponentsOnEntities(
            const std::unordered_set<entt::entity>& entities,
            Core::Registry& registry,
            const std::unordered_map<uint64_t, uint64_t>& uuidRemap)
        {
            // Skip UUID component itself (already handled in CollectUUIDsFromEntities)
            if constexpr (std::is_same_v<C, UUID>) {
                return;
            }

            for (auto entity : entities) {
                if (registry.all_of<C>(entity)) {
                    C& component = registry.get<C>(entity);
                    // Use reflection to find and update UUID fields in the component
                    RemapUUIDFieldsInComponent(component, uuidRemap);
                }
            }
        }

        // Generic function to remap UUID fields in any component using reflection
        template<typename C>
        void RemapUUIDFieldsInComponent(
            C& component,
            const std::unordered_map<uint64_t, uint64_t>& uuidRemap)
        {
            // Get meta type for the component
            auto metaType = entt::resolve<C>();
            if (!metaType) {
                return; // Component not reflected
            }

            // Get the resolved UUID type
            auto uuidType = entt::resolve<UUID>();
            if (!uuidType) {
                return; // UUID type not reflected
            }

            // Iterate over all data members (fields) of the component
            for (auto&& [id, metaData] : metaType.data()) {
                // Check if this field is of type Core::UUID
                if (metaData.type() == uuidType) {
                    // Use set() to modify the value through the metadata
                    // First get the current value
                    entt::meta_any currentValue = metaData.get(component);
                    if (!currentValue) {
                        continue;
                    }

                    // Try to get the UUID value
                    if (const auto* currentUUID = currentValue.try_cast<UUID>()) {
                        if (currentUUID->value != 0) {
                            auto it = uuidRemap.find(currentUUID->value);
                            if (it != uuidRemap.end()) {
                                // UUID references an entity within the selection - create new UUID and set it
                                UUID newUUID(it->second);
                                metaData.set(component, newUUID);
                            }
                            // If UUID is not in the remap table, it references something outside
                            // the selection, so we leave it unchanged
                        }
                    }
                }
            }
        }

        // Helper: Resolve component name hash to actual component type ID
        static entt::id_type resolve_component_type(entt::id_type nameHash) {
            auto meta_type = entt::resolve(nameHash);
            if (!meta_type) {
                throw std::runtime_error("Component type not found in reflection system");
            }
            return meta_type.info().hash();
        }
    };

}