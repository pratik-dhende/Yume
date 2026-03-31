#pragma once

#include "Component.h"

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace Yume {

class Entity {
public:
	explicit Entity(const std::string& name) : m_name(name) {}

	const std::string& GetName() const { return m_name; }
	bool IsActive() const { return m_active; }
	void SetActive(bool isActive) { m_active = isActive; }

	void Initialize() {
		for (auto& component : m_components) {
			component->Initialize();
		}
	}

	void Update(float deltaTime) {
		if (!m_active) {
			return;
		}

		for (auto& component : m_components) {
			component->Update(deltaTime);
		}
	}

	void Render() {
		if (!m_active) {
			return;
		}
		for (auto& component : m_components) {
			component->Render();
		}
	}

	template<typename T, typename... Args>
	T* AddComponent(Args&&... args) {
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

		auto typeId = Component::GetTypeId<T>();
		auto it = m_componentsMap.find(typeId);
		if (it != m_componentsMap.end()) {
			return static_cast<T*>(it->second);
		}

		auto component = std::make_unique<T>(std::forward<Args>(args)...);
		component->SetOwner(this);
		auto componentPtr = component.get();
		m_components.push_back(std::move(component));
		m_componentsMap[typeId] = componentPtr;

		return componentPtr;
	}

	template<typename T>
	T* GetComponent() const {
		auto it = m_componentsMap.find(Component::GetTypeId<T>());
		return it != m_componentsMap.end() ? static_cast<T*>(it->second) : nullptr;
	}

	template<typename T>
	bool RemoveComponent() {
		auto typeId = Component::GetTypeId<T>();

		auto it = m_componentsMap.find(typeId);
		if (it == m_componentsMap.end()) {
			return false;
		}

		auto componentPtr = it->second;
		m_componentsMap.erase(typeId);
		for (auto componentIt = m_components.begin(); componentIt != m_components.end(); ++componentIt) {
			if (componentIt->get() == componentPtr) {
				m_components.erase(componentIt);
				return true;
			}
		}
		return false;
	}

private:
	std::string m_name;
	bool m_active = true;
	std::vector<std::unique_ptr<Component>> m_components;
	std::unordered_map<size_t, Component*> m_componentsMap;
};

}