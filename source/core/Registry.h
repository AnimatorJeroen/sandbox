#pragma once
#include <entt/entt.hpp>
#include <entt/meta/meta.hpp>
#include <map>
#include "core/UUID.h"

namespace Core
{
	class Registry
	{
	public:
		// Entity creation and destruction
		//entt::entity createWithoutUUID() { return _registry.create(); }
		entt::entity Create() { auto e = _registry.create(); auto uuid = _registry.emplace<UUID>(e); _entityToUUID[e] = uuid; return e; }
		
		void destroy(entt::entity entity) { _registry.destroy(entity);  _entityToUUID.erase(entity); }
		void clear() { _entityToUUID.clear(); return _registry.clear(); }
		bool valid(entt::entity entity) const { return _registry.valid(entity); }

		// Entity version and identifiers
		auto current(entt::entity entity) const { return _registry.current(entity); }
		
		// // Implicit conversion to entt::registry& for compatibility
		//operator entt::registry&() { return _registry; }
		//operator const entt::registry&() const { return _registry; }

		// Component emplacement
		template<typename Component, typename... Args>
		decltype(auto) emplace(entt::entity entity, Args&&... args)
		{
			return _registry.emplace<Component>(entity, std::forward<Args>(args)...);
		}

		// Component emplacement or replace
		template<typename Component, typename... Args>
		decltype(auto) emplace_or_replace(entt::entity entity, Args&&... args)
		{
			return _registry.emplace_or_replace<Component>(entity, std::forward<Args>(args)...);
		}

		// Component replacement
		template<typename Component, typename... Args>
		decltype(auto) replace(entt::entity entity, Args&&... args)
		{
			return _registry.replace<Component>(entity, std::forward<Args>(args)...);
		}

		// Component patching
		template<typename Component, typename Func>
		void patch(entt::entity entity, Func&& func)
		{
			_registry.patch<Component>(entity, std::forward<Func>(func));
		}

		// Component removal
		template<typename Component>
		size_t remove(entt::entity entity)
		{
			return _registry.remove<Component>(entity);
		 }

		// Bulk component removal (with iterators)
		template<typename... Component, typename It>
		void remove(It first, It last)
		{
			_registry.remove<Component...>(first, last);
		}

		// Erase component (alternative name)
		template<typename Component>
		size_t erase(entt::entity entity)
		{
			return _registry.erase<Component>(entity);
		}

		// Component access - single component
		template<typename Component>
		decltype(auto) get(entt::entity entity)
		{
			return _registry.get<Component>(entity);
		}

		template<typename Component>
		decltype(auto) get(entt::entity entity) const
		{
			return _registry.get<Component>(entity);
		}

		// Multi-component access (for structured bindings with 2+ components)
		template<typename Component1, typename Component2, typename... Rest>
		decltype(auto) get(entt::entity entity)
		{
			return _registry.get<Component1, Component2, Rest...>(entity);
		}

		template<typename Component1, typename Component2, typename... Rest>
		decltype(auto) get(entt::entity entity) const
		{
			return _registry.get<Component1, Component2, Rest...>(entity);
		}

		 // Try get (returns pointer, nullptr if component doesn't exist)
		template<typename Component>
		decltype(auto) try_get(entt::entity entity)
		{
			return _registry.try_get<Component>(entity);
		}

		template<typename Component>
		decltype(auto) try_get(entt::entity entity) const
		{
			return _registry.try_get<Component>(entity);
		}

		// Component check
		template<typename... Components>
		bool all_of(entt::entity entity) const
		{
			return _registry.all_of<Components...>(entity);
		}

		template<typename... Components>
		bool any_of(entt::entity entity) const
		{
			return _registry.any_of<Components...>(entity);
		}

		 // Storage access
		template<typename Component>
		decltype(auto) storage()
		{
			return _registry.storage<Component>();
		}

		template<typename Component>
		decltype(auto) storage() const
		{
			return _registry.storage<Component>();
		}

		// Storage access by type id
		decltype(auto) storage(entt::id_type id)
		{
			return _registry.storage(id);
		}

		decltype(auto) storage(entt::id_type id) const
		{
			return _registry.storage(id);
		}

		// Orphan check
		bool orphan(entt::entity entity) const
		{
			return _registry.orphan(entity);
		}

		// View creation
		template<typename... Components>
		auto view()
		{
			return _registry.view<Components...>();
		}

		template<typename... Components>
		auto view() const
		{
			return _registry.view<Components...>();
		}

		template<typename... Components, typename... Exclude>
		auto view(entt::exclude_t<Exclude...> excl)
		{
			return _registry.view<Components...>(excl);
		}

		template<typename... Components, typename... Exclude>
		auto view(entt::exclude_t<Exclude...> excl) const
		{
			return _registry.view<Components...>(excl);
		}

		// Group creation (runtime groups)
		template<typename... Owned, typename... Get, typename... Exclude>
		decltype(auto) group(entt::get_t<Get...> = {}, entt::exclude_t<Exclude...> = {})
		{
			return _registry.template group<Owned...>(entt::get<Get...>, entt::exclude<Exclude...>);
		}

		template<typename... Owned, typename... Get, typename... Exclude>
		decltype(auto) group(entt::get_t<Get...> = {}, entt::exclude_t<Exclude...> = {}) const
		{
			return _registry.template group<Owned...>(entt::get<Get...>, entt::exclude<Exclude...>);
		}

		// Sorting
		template<typename Component, typename Compare>
		void sort(Compare&& compare)
		{
			_registry.sort<Component>(std::forward<Compare>(compare));
		}

		template<typename To, typename From>
		void sort()
		{
			_registry.sort<To, From>();
		}

		// Capacity and size
		template<typename Component>
		auto size() const
		{
			return _registry.storage<Component>().size();
		}

		template<typename Component>
		auto capacity() const
		{
			return _registry.storage<Component>().capacity();
		}

		// Reserve
		template<typename Component>
		void reserve(std::size_t cap)
		{
			_registry.storage<Component>().reserve(cap);
		}

		// Direct registry access (for advanced use cases)
		entt::registry& GetRegistry() { return _registry; }
		const entt::registry& GetRegistry() const { return _registry; }

		 // Context access (for storing global data like Scene pointer)
		decltype(auto) ctx() { return _registry.ctx(); }
		decltype(auto) ctx() const { return _registry.ctx(); }

		// UUID mapping functions
		void MapEntityToUUID(entt::entity entity, const UUID& uuid)
		{
			_entityToUUID[entity] = uuid;
		}

		UUID GetUUID(entt::entity entity) const
		{
			auto it = _entityToUUID.find(entity);
			return it != _entityToUUID.end() ? it->second : UUID::Null;
		}

		entt::entity GetEntityByUUID(const UUID& uuid) const
		{
			for (const auto& [entity, entityUUID] : _entityToUUID)
			{
				if (entityUUID == uuid)
					return entity;
			}
			return entt::null;
		}

		// Only called internally by applicator after bringing back a UUID component, or pasting
		void ModifyUUID(entt::entity entity, UUID newUUID)
		{
			_entityToUUID[entity] = newUUID;
			_registry.get<UUID>(entity) = newUUID;
		}


	private:
		entt::registry _registry;
		std::map<entt::entity, UUID> _entityToUUID;
	};
}