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

            // Build UUID remap table: old UUID -> new UUID
            // This maps all UUIDs in the selection to fresh UUIDs
            std::unordered_map<uint64_t, uint64_t> uuidRemap;
            
            // First pass: collect all UUIDs from entities in the selection and generate new ones
            CollectAndRemapUUIDs(archive, uuidRemap);
            
            // Second pass: remap any UUID fields in components that reference UUIDs within the selection
            RemapUUIDsInArchive(archive, uuidRemap);

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

            // Get the archive from clipboard
            const auto* archive = _clipboard.GetEntitiesArchive<Cs...>();
            if (!archive || archive->entities.empty()) {
                return; // Nothing to paste
            }

            // Use unified restore function to create entities and restore components
            // The archive already has UUIDs remapped from the copy operation
            auto newEntities = ArchiveHelpers::RestoreEntitiesAndComponents(*registry, *archive);

            // Capture the creation for undo/redo
            CaptureCreate(newEntities);
        }

        // Collect all UUIDs from the UUID component in the archive
        // and generate new UUIDs for them (builds the remap table)
        template<typename... Cs>
        void CollectAndRemapUUIDs(
            SelectionArchive<Cs...>& archive,
            std::unordered_map<uint64_t, uint64_t>& uuidRemap)
        {
            // Use fold expression to search for UUID component type
            (CollectAndRemapUUIDsFromStorage<Cs>(archive, uuidRemap), ...);
        }

        // Helper to collect and remap UUIDs from a specific component storage
        template<typename C, typename... Cs>
        void CollectAndRemapUUIDsFromStorage(
            SelectionArchive<Cs...>& archive,
            std::unordered_map<uint64_t, uint64_t>& uuidRemap)
        {
            // Special handling for UUID component type
            if constexpr (std::is_same_v<C, UUID>) {
                auto& storage = std::get<std::vector<std::pair<entt::entity, C>>>(archive.storages);
                for (auto& [entity, uuidComponent] : storage) {
                    uint64_t oldUUID = uuidComponent.value;
                    uint64_t newUUID = UUID::generate();
                    
                    // Store mapping
                    uuidRemap[oldUUID] = newUUID;
                    
                    // Update the UUID component in the archive with the new UUID
                    uuidComponent.value = newUUID;
                }
            }
        }

        // Remap any UUID fields in all components in the archive
        // Uses reflection to find UUID-typed fields
        template<typename... Cs>
        void RemapUUIDsInArchive(
            SelectionArchive<Cs...>& archive,
            const std::unordered_map<uint64_t, uint64_t>& uuidRemap)
        {
            // Process each component type
            (RemapUUIDsInComponentStorage<Cs>(archive, uuidRemap), ...);
        }

        // Helper to remap UUIDs in a specific component storage
        template<typename C, typename... Cs>
        void RemapUUIDsInComponentStorage(
            SelectionArchive<Cs...>& archive,
            const std::unordered_map<uint64_t, uint64_t>& uuidRemap)
        {
            // Skip UUID component itself (already handled in CollectAndRemapUUIDs)
            if constexpr (std::is_same_v<C, UUID>) {
                return;
            }

            auto& storage = std::get<std::vector<std::pair<entt::entity, C>>>(archive.storages);
            
            for (auto& [entity, component] : storage) {
                // Use reflection to find and update UUID fields in the component
                RemapUUIDFieldsInComponent(component, uuidRemap);
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

            // Iterate over all data members (fields) of the component
            for (auto&& [id, metaData] : metaType.data()) {
                // Check if this field is of type Core::UUID
                if (metaData.type() == entt::resolve<UUID>()) {
                    // Get the field value as a meta_any
                    entt::meta_any fieldHandle = metaData.get(component);
                    if (!fieldHandle) {
                        continue;
                    }

                    // Cast to UUID and check if it needs remapping
                    UUID* uuidPtr = fieldHandle.try_cast<UUID>();
                    if (uuidPtr && uuidPtr->value != 0) {
                        auto it = uuidRemap.find(uuidPtr->value);
                        if (it != uuidRemap.end()) {
                            // UUID references an entity within the selection - remap it
                            UUID newUUID(it->second);
                            metaData.set(component, newUUID);
                        }
                        // If UUID is not in the remap table, it references something outside
                        // the selection, so we leave it unchanged
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