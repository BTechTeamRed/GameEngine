#pragma once

#include "entt.h"
#include "Engine/Core.h"
#include <Engine/Utilities/DeltaTime.h>
#include <string>

#include "Engine/Physics/PhysicsSystem.h"
namespace Engine
{
    class Entity;

    // Scene keeps a registry of Entities that are then rendered and processed as part of the game loop.
    class ENGINE_API Scene
    {
    public:
        bool m_closeScene = false;
        bool m_switch{ false };
        int score = 1;
        std::string m_nextScene;
		std::vector<std::string> m_sceneList;
        
        #pragma region Entity Management
        // Registry is a container to hold entities
        //Made public to allow for GLFW callbacks to access entities
        entt::registry m_registry;

        // Function to create an entity and add it to the scene
        // Takes in a tag as a string for input
        Entity createEntity(std::string tag);

        // Gets a view of entities with the defined components.
        template<typename... Components>
        auto getEntities()
        {
            return m_registry.view<Components...>();
        }
        #pragma endregion
        
        #pragma region Runtime Functions
        // Executes actions at the start of runtime
        void onRuntimeStart();

        // Executes actions when runtime stops
        void onRuntimeStop();

        // Executes actions every time runtime is updated (every frame).
        void onRuntimeUpdate(const DeltaTime& dt);

        void swapScene(const std::string& other);
        #pragma endregion

        /// <summary>
        /// Exposure of the picking system to scripts via m_entity.getScene()
        /// </summary>
        /// <param name="x">X-cooridnate on screen</param>
        /// <param name="y">Y-coordinate on screen</param>
        /// <returns></returns>
        std::list<Entity*> pick(float x, float y);

        std::string m_name;
	
    private:
        DeltaTime m_deltaTime{ 0 };
        PhysicsSystem* m_physics;

		// Creates a new physics world and inserts physics entities
        void createPhysics();

        void runEntityScripts(const DeltaTime& dt);

        void renderScene();
        
        friend class Entity;
        friend class Serializer;
        
    };
}
