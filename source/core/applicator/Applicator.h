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
            auto newEntities = ArchiveHelpers::RestoreEntitiesAndComponents(*registry, *archive);

            // Build a map from old UUIDs to new UUIDs
            std::unordered_map<uint64_t, uint64_t> uuidRemap;
            
            // Generate NEW UUIDs for all pasted entities (don't copy old UUIDs)
            for (auto entity : newEntities) {
                // Store the old UUID before removing it
                uint64_t oldUUID = 0;
                if (registry->all_of<UUID>(entity)) {
                    oldUUID = registry->get<UUID>(entity).value;
                    registry->remove<UUID>(entity);
                }
                
                // Add a fresh UUID
                UUID newUUID = registry->emplace<UUID>(entity);
                
                // Store the mapping from old UUID to new UUID
                if (oldUUID != 0) {
                    uuidRemap[oldUUID] = newUUID.value;
                }
            }
            
            // Update Parent components to reference new UUIDs
            // This iterates through all component types and updates any that have a parentUUID field
            (UpdateIfParentComponent<Cs>(newEntities, uuidRemap, registry), ...);

            // Capture the creation for undo/redo
            CaptureCreate(newEntities);
        }

        // Type trait to detect if a type has a parentUUID member of type Core::UUID
        template<typename T, typename = void>
        struct HasParentUUID : std::false_type {};
        
        template<typename T>
        struct HasParentUUID<T, std::void_t<decltype(std::declval<T>().parentUUID)>> : 
            std::is_same<decltype(std::declval<T>().parentUUID), Core::UUID> {};
        
        // Helper template to update component if it's a Parent type
        template<typename C>
        void UpdateIfParentComponent(
            const std::unordered_set<entt::entity>& entities,
            const std::unordered_map<uint64_t, uint64_t>& uuidRemap,
            entt::registry* registry)
        {
            // Check if C has a member called 'parentUUID' of type Core::UUID
            // This is a compile-time check that identifies the Parent component
            if constexpr (HasParentUUID<C>::value) {
                for (auto entity : entities) {
                    if (registry->all_of<C>(entity)) {
                        C& comp = registry->get<C>(entity);
                        
                        // Check if parent UUID is valid and in the remap table
                        if (comp.parentUUID.value != 0) {
                            auto it = uuidRemap.find(comp.parentUUID.value);
                            if (it != uuidRemap.end()) {
                                // Parent was also pasted, update to new UUID
                                comp.parentUUID = UUID(it->second);
                            } else {
                                // Parent was not pasted (external reference), clear the parent
                                // to avoid referencing an entity that doesn't exist in paste context
                                comp.parentUUID = UUID(0);
                            }
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