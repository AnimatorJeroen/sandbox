#pragma once

#include <core/memory/SelectionArchive.h>

namespace Core {
    // Enum to track the current clipboard context type
    enum class ClipboardType {
        None,
        Entities,
        // Future clipboard types can be added here:
        // Text,
        // Components,
        // etc.
    };

    // Base class for type-erased clipboard storage
    struct ClipboardStorageBase {
        virtual ~ClipboardStorageBase() = default;
    };

    // Templated storage for specific archive types
    template<typename... Cs>
    struct ClipboardStorage : ClipboardStorageBase {
        SelectionArchive<Cs...> archive;

        explicit ClipboardStorage(SelectionArchive<Cs...>&& arch)
            : archive(std::move(arch)) {
        }
    };

    // Clipboard class that holds different context types
    template<typename ComponentTypes>
    class Clipboard {
    public:
        Clipboard() = default;

        // Store entities selection archive
        template<typename... Cs>
        void StoreEntities(SelectionArchive<Cs...>&& archive) {
            _entitiesClipboard = std::make_unique<ClipboardStorage<Cs...>>(std::move(archive));
            _activeType = ClipboardType::Entities;
        }

        // Check if clipboard has entities data
        [[nodiscard]] bool HasEntities() const noexcept {
            return _activeType == ClipboardType::Entities && _entitiesClipboard != nullptr;
        }

        // Check if clipboard is empty
        [[nodiscard]] bool IsEmpty() const noexcept {
            return _activeType == ClipboardType::None;
        }

        // Get the active clipboard type
        [[nodiscard]] ClipboardType GetActiveType() const noexcept {
            return _activeType;
        }

        // Clear the clipboard
        void Clear() {
            _entitiesClipboard.reset();
            _activeType = ClipboardType::None;
        }

        // Get the stored entities archive (for future paste operations)
        template<typename... Cs>
        const SelectionArchive<Cs...>* GetEntitiesArchive() const {
            if (_activeType != ClipboardType::Entities || !_entitiesClipboard) {
                return nullptr;
            }
            auto* storage = dynamic_cast<ClipboardStorage<Cs...>*>(_entitiesClipboard.get());
            return storage ? &storage->archive : nullptr;
        }

    private:
        ClipboardType _activeType = ClipboardType::None;
        std::unique_ptr<ClipboardStorageBase> _entitiesClipboard;

        // Future clipboard contexts can be added here:
        // std::unique_ptr<TextClipboard> _textClipboard;
        // std::unique_ptr<ComponentClipboard> _componentClipboard;
    };
}