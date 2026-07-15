#pragma once 

#include <string>
#include <chrono>

class Entity;

class Component
{
    protected:
        Entity *owner = nullptr;
        std::string name;
        bool active = true;

    public:

        explicit Component(const std::string &componentName = "Component") : name(componentName) {}

        virtual ~Component() = default;

        virtual void initialize() {}

        virtual void update(std::chrono::milliseconds deltaTime) {}

        virtual void render() {}

        void SetOwner(Entity *entity)
        {
            owner = entity;
        }

        Entity *GetOwner() const
        {
            return owner;
        }

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
};