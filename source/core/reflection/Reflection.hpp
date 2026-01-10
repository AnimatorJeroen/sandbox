#pragma once

#include "../vendor/include/entt/entt.hpp"
#include "../vendor/include/entt/meta/meta.hpp"
#include <variant>

// ===========================================================================
// Header-Only Reflection System
// ===========================================================================
// This reflection system is completely header-only and type-agnostic.
// It works with any std::variant-based ValueTypes through template parameters.
// No user types or application-specific types are referenced here.
// ===========================================================================

// ----- Path element: either a field name (id_type) or an array/vector index (size_t)
using PathElement = std::variant<entt::id_type, size_t>;
// Include Path definition
#include "reflectionPathParser.hpp"

namespace Core {

// ----- meta_any <-> Value helpers (templated for different ValueTypes)
// Users MUST provide their own ValueTypes - no default implementations
	template<typename ValueTypes>
	ValueTypes ToValue(const entt::meta_any& any);

	template<typename ValueTypes>
	entt::meta_any FromValue(const ValueTypes& v);

	// ----- Walk path via meta IDs and indices
	// e.g., chain = {entt::id_type("Position"), entt::id_type("x")} for fields
	// e.g., chain = {entt::id_type("Weights"), size_t(0)} for vector[0]
	// Uses sentinel-terminated fixed-size array (max 16 elements)
	template<typename ValueTypes>
	ValueTypes GetByPath(entt::meta_any inst, const reflection::Path& chain);

	// ----- Set value by path
	// Uses sentinel-terminated fixed-size array (max 16 elements)
	template<typename ValueTypes>
	void SetByPath(entt::meta_any& inst, const reflection::Path& chain, const ValueTypes& v);
}

// ===========================================================================
// Template implementations must be in header for header-only library support
// ===========================================================================

#include "ReflectionImpl.hpp"
