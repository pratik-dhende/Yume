#pragma once

namespace Yume {

class ComponentTypeIdSystem {
public:
	template<typename T>
	static size_t GetTypeId() {
		static size_t typeId = m_nextTypeId++;
		return typeId;
	}

private:
	static size_t m_nextTypeId;
};

size_t ComponentTypeIdSystem::m_nextTypeId = 0;

class Entity;

class Component {
public:
	enum class State {
		Uninitialized,
		Initializing,
		Active,
		Destroying,
		Destroyed
	};

public:
	virtual ~Component() {
		if (m_state != State::Destroyed) {
			m_state = State::Destroying;
			OnDestroy();
			m_state = State::Destroyed;
		}
	}

	void Initialize() {
		if (m_state == State::Uninitialized) {
			m_state = State::Initializing;
			OnInitialize();
			m_state = State::Active;
		}
	}

	void Destroy() {
		if (m_state == State::Active) {
			m_state = State::Destroying;
			OnDestroy();
			m_state = State::Destroyed;
		}
	}

	template<typename T>
	static size_t GetTypeId() {
		return ComponentTypeIdSystem::GetTypeId<T>();
	}

	bool IsActive() const { return m_state == State::Active; }

	void SetOwner(Entity* entity) { m_owner = entity; }
	Entity* GetOwner() const { return m_owner; }

protected:
	State m_state = State::Uninitialized;
	Entity* m_owner = nullptr;

protected:
	virtual void OnInitialize() {}
	virtual void OnDestroy() {}
	virtual void Update(float deltaTime) {}
	virtual void Render() {}

	friend class Entity; // Allow Entity to call protected methods
};

}