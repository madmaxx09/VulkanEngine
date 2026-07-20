#pragma once

#include <algorithm>
#include <chrono>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "component.h"

class Entity
{
	private:
		std::string name;
		bool active = true;
		std::vector<std::unique_ptr<Component>> components;

	public:

		explicit Entity(const std::string &entityName) :
	    	name(entityName)
		{}

		virtual ~Entity() = default;

		const std::string &GetName() const
		{
			return name;
		}

		bool IsActive() const
		{
			return active;
		}

		void SetActive(bool isActive)
		{
			active = isActive;
		}

		void Initialize();

		void Update(std::chrono::milliseconds deltaTime);

		void Render();

		template <typename T, typename... Args>
		T *AddComponent(Args &&...args)
		{
			static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

			// Create the component
			auto component    = std::make_unique<T>(std::forward<Args>(args)...);
			T   *componentPtr = component.get();

			// Set the owner
			componentPtr->SetOwner(this);

			// Add to the vector for ownership and iteration
			components.push_back(std::move(component));

			// Initialize the component
			componentPtr->initialize();

			return componentPtr;
		}

		template <typename T>
		T *GetComponent() const
		{
			static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

			// Search from the back to preserve previous behavior of returning the last-added component of type T
			for (auto it = components.rbegin(); it != components.rend(); ++it)
			{
				if (auto *casted = dynamic_cast<T *>(it->get()))
				{
					return casted;
				}
			}
			return nullptr;
		}

		template <typename T>
		bool RemoveComponent()
		{
			static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

			for (auto it = components.rbegin(); it != components.rend(); ++it)
			{
				if (dynamic_cast<T *>(it->get()) != nullptr)
				{
					components.erase(std::next(it).base());
					return true;
				}
			}

			return false;
		}

		template <typename T>
		bool HasComponent() const
		{
			static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
			return GetComponent<T>() != nullptr;
		}

		//std::vector<std::unique_ptr<Component>> &getComponents()
};