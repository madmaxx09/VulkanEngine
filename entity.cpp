#include "entity.h"

void Entity::Initialize()
{
    for (auto &component : components)
    {
        component->initialize();
    }
}

void Entity::Update(std::chrono::milliseconds deltaTime)
{
	if (!active)
		return;

	for (auto &component : components)
	{
		if (component->IsActive())
		{
			component->update(deltaTime);
		}
	}
}

void Entity::Render()
{
	if (!active)
		return;

	for (auto &component : components)
	{
		if (component->IsActive())
		{
			component->render();
		}
	}
}