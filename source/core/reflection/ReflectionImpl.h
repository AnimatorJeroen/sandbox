#pragma once

#include "../vendor/include/entt/entt.hpp"
#include <iostream>
#include <stdexcept>
#include <variant>
#include <type_traits>

// ===========================================================================
// Template Implementation for Reflection Functions
// ===========================================================================
// This file contains the template implementations that need to be in headers
// for the undo system to work as a header-only library.
//
// NOTE: This file does NOT include Types.hpp or any user-defined types.
// It is completely generic and works with any std::variant-based ValueTypes.
// ===========================================================================

namespace Core {


    // ----- Helper: Try to convert meta_any to a specific variant alternative
    namespace detail {
        template<typename T>
        struct meta_any_to_variant_alternative {
            static bool try_convert(const entt::meta_any& any, auto& result) {
                if (any.type() == entt::resolve<T>()) {
                    result = any.cast<T>();
                    return true;
                }
                return false;
            }
        };

        // Recursive template to try each variant alternative
        template<typename ValueTypes, std::size_t Index = 0>
        bool try_all_alternatives(const entt::meta_any& any, ValueTypes& result) {
            if constexpr (Index < std::variant_size_v<ValueTypes>) {
                using Alternative = std::variant_alternative_t<Index, ValueTypes>;

                if (any.type() == entt::resolve<Alternative>()) {
                    result = ValueTypes{ any.cast<Alternative>() };
                    return true;
                }

                // Try next alternative
                return try_all_alternatives<ValueTypes, Index + 1>(any, result);
            }
            return false;
        }
    }

    // ----- meta_any <-> Value helpers (fully templated implementation)
    template<typename ValueTypes>
    ValueTypes ToValue(const entt::meta_any& any) {
        // This is a fully generic implementation that works with any std::variant-based ValueTypes
        // It automatically tries to convert to each alternative type in the variant

        static_assert(std::is_same_v<ValueTypes, std::decay_t<ValueTypes>>,
            "ValueTypes must be a non-cv-qualified type");

        // Ensure ValueTypes is a variant
        static_assert([]<typename... Ts>(std::variant<Ts...>*) { return true; }
        (static_cast<ValueTypes*>(nullptr)),
            "ValueTypes must be a std::variant");

        ValueTypes result;
        if (detail::try_all_alternatives(any, result)) {
            return result;
        }

        // If no alternative matched, throw an error
        throw std::runtime_error("Unsupported type in ToValue(): " + std::string(any.type().info().name()));
    }

    template<typename ValueTypes>
    entt::meta_any FromValue(const ValueTypes& v) {
        // For std::variant-based ValueTypes
        // Uses std::visit to generically handle any variant type

        return std::visit([](auto&& arg) -> entt::meta_any {
            return entt::meta_any{ arg };
            }, v);
    }

    // ----- Walk path via PathElements (field names or indices)
    // Uses sentinel-terminated fixed-size array
    template<typename ValueTypes>
    ValueTypes GetByPath(entt::meta_any inst, const reflection::Path& chain) {
        entt::meta_any cur = inst.as_ref();

        for (size_t i = 0; i < reflection::MAX_PATH_ELEMENTS && !reflection::IsSentinel(chain[i]); ++i) {
            const auto& element = chain[i];

            if (std::holds_alternative<entt::id_type>(element)) {
                // Navigate through a field
                const auto id = std::get<entt::id_type>(element);
                auto data = cur.type().data(id);
                if (!data) [[unlikely]] throw std::runtime_error("Missing field id");
                cur = data.get(cur).as_ref();
                if (!cur) [[unlikely]] throw std::runtime_error("Failed to read field");
            }
            else if (std::holds_alternative<size_t>(element)) {
                // Navigate through an index (sequence container)
                const auto index = std::get<size_t>(element);
                auto seq_container = cur.as_sequence_container();
                if (!seq_container) [[unlikely]] {
                    throw std::runtime_error("Element is not a sequence container");
                }

                if (index >= seq_container.size()) [[unlikely]] {
                    throw std::runtime_error("Index out of bounds");
                }

                auto it = seq_container.begin();
                std::advance(it, index);
                cur = *it;
                cur = cur.as_ref();
                if (!cur) [[unlikely]] throw std::runtime_error("Failed to read element at index");
            }
        }

        return ToValue<ValueTypes>(cur);
    }

    template<typename ValueTypes>
    void SetByPath(entt::meta_any& inst,
        const reflection::Path& chain,
        const ValueTypes& v)
    {
        entt::meta_any cur = inst.as_ref();

        // Find the length (last non-sentinel element)
        size_t length = reflection::PathLength(chain);

        if (length == 0) [[unlikely]] throw std::runtime_error("Empty path");

        // descend to parent of final element
        for (size_t i = 0; i < length - 1; ++i) {
            const auto& element = chain[i];

            if (std::holds_alternative<entt::id_type>(element)) {
                // Navigate through a field
                const auto id = std::get<entt::id_type>(element);
                auto data = cur.type().data(id);
                if (!data) [[unlikely]] throw std::runtime_error("Missing intermediate field id");

                cur = data.get(cur).as_ref();
                if (!cur) [[unlikely]] throw std::runtime_error("Failed to read intermediate field");
            }
            else if (std::holds_alternative<size_t>(element)) {
                // Navigate through an index
                const auto index = std::get<size_t>(element);
                auto seq_container = cur.as_sequence_container();
                if (!seq_container) [[unlikely]] {
                    throw std::runtime_error("Intermediate element is not a sequence container");
                }

                if (index >= seq_container.size()) [[unlikely]] {
                    throw std::runtime_error("Index out of bounds");
                }

                auto seq_it = seq_container.begin();
                std::advance(seq_it, index);
                cur = *seq_it;
                cur = cur.as_ref();
                if (!cur) [[unlikely]] throw std::runtime_error("Failed to read element at index");
            }
        }

        // Now set the final element
        const auto& final_element = chain[length - 1];

        if (std::holds_alternative<entt::id_type>(final_element)) {
            // Setting a field
            const auto finalId = std::get<entt::id_type>(final_element);
            auto final = cur.type().data(finalId);
            if (!final) [[unlikely]] throw std::runtime_error("Missing final field id");

            entt::meta_any newAny = FromValue<ValueTypes>(v);
            if (!final.set(cur, newAny)) [[unlikely]] {
                throw std::runtime_error("Write failed");
            }
        }
        else if (std::holds_alternative<size_t>(final_element)) {
            // Setting an element in a sequence container using operator[]
            const auto index = std::get<size_t>(final_element);
            auto seq_container = cur.as_sequence_container();
            if (!seq_container) [[unlikely]] {
                throw std::runtime_error("Final element parent is not a sequence container");
            }

            if (index >= seq_container.size()) [[unlikely]] {
                throw std::runtime_error("Index out of bounds");
            }

            // Use operator[] to get a reference to the element
            entt::meta_any element = seq_container[index];
            entt::meta_any newAny = FromValue<ValueTypes>(v);

            // Assign the new value to the element
            if (!element.assign(newAny)) [[unlikely]] {
                throw std::runtime_error("Write failed on sequence element");
            }
        }
    }
}