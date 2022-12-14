#pragma once
#include "Engine/Core.h"
#include "Engine/SceneBuilder/Entity.h"
#include "Engine/SceneBuilder/Components.h" //include facilitates use of component in child scripts
#include "Engine/Utilities/Log.h"

namespace Engine
{
	class DeltaTime;

	// Polymorphic class that binds to <ScriptComponent>.
	// classes inheriting from ScriptableBehavior essentially give entities different behavior during the Scene loop.
	class ENGINE_API ScriptableBehavior
	{
	public:
		virtual ~ScriptableBehavior() = default;

		//Returns a component T if the entity has one, otherwise it returns nullptr.
		template<typename T>
		T& getComponent()
		{
			return m_entity.getComponent<T>();
		}

		//This function should return the name of the script header without the extension.
		//Failing to override this function will lead to errors in script serialization.
		//i.e. "MoveScript.h" should override this function to return "MoveScript".
		virtual std::string getScriptName() = 0;

	protected:
		//gets called once after Entities are serialized and created, in Scene::createEntities().
		virtual void onCreate() {}

		//gets called before the entity is destroyed.
		//currently unused since we are not destroying entities until the game is exited.
		virtual void onDestroy() {}

		//Runs on a loop before rendering
		virtual void onUpdate(const DeltaTime& dt) {}

		//Runs after all other Entity onUpdates have been called and after rendering
		virtual void onLateUpdate(const DeltaTime &dt) {}

		//runs when enabled/disabled
		virtual void onDisable() {} 
		virtual void onEnable() {}

		// Gets a view of entities with the defined components.
		template<typename... Components>
		auto getEntities()
		{
			return m_entity.m_scene->getEntities<Components...>();
		}

		bool doesEntityExist(const std::string& tagName)
		{
			auto entities = getEntities<TagComponent>();
			for (auto [entity, tag] : entities.each())
			{
				if (tag.tag == tagName)
				{
					return true;
				}
			}

			return false;
		}

		Entity getEntity(const std::string& tagName)
		{
			auto entities = getEntities<TagComponent>();
			for (auto [entity, tag] : entities.each())
			{
				if (tag.tag == tagName)
				{
					return Entity{ entity, m_entity.m_scene };
				}
			}

			//DefinitelyAssert
			return Entity{entt::entity() , nullptr};
		}

		void enable() { m_enabled = true; this->onEnable(); }
		void disable() { m_enabled = false; this->onDisable(); }

		const std::string& toString() { return m_entity.getName(); }
		const std::string& getName() { return m_entity.getName(); }

		//TODO add event functionality
	   
		Entity m_entity;
	private:
		//A disabled entity will not execute onUpdate() or onLateUpdate();
		bool m_enabled{true};

		friend class Scene;
	};
}

