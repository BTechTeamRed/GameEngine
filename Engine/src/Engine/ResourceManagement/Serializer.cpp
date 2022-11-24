#include "Serializer.h"

#include <iostream>

#include "Engine/SceneBuilder/Entity.h"
#include "Engine/Utilities/Log.h"
#include "Engine/SceneBuilder/Scene.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/GeometryFactory.h"
#include "Engine/ResourceManagement/ResourceManager.h"
#include "Engine/ResourceManagement/ScriptSerializer.h"
#include "Engine/Scripts/ScriptableBehavior.h"

namespace glm
{
	#pragma region Json Serialization
	//templates from nlohmann. Serializes/deserializes custom types
	void to_json(nlohmann::json& j, const vec2& vec)
	{
		j = { { "x", vec.x }, { "y", vec.y } };
	}

	void from_json(const nlohmann::json& j, vec2& vec)
	{
		vec.x = j.at("x").get<float>();
		vec.y = j.at("y").get<float>();
	}

	void to_json(nlohmann::json& j, const vec3& vec)
	{
		j = { { "x", vec.x }, { "y", vec.y }, { "z", vec.z } };
	}

	void from_json(const nlohmann::json& j, vec3& vec)
	{
		vec.x = j.at("x").get<float>();
		vec.y = j.at("y").get<float>();
		vec.z = j.at("z").get<float>();
	}

	void to_json(nlohmann::json& j, const vec4& vec)
	{
		j = { { "r", vec.r }, { "g", vec.g }, { "b", vec.b }, { "a", vec.a } };
	}

	void from_json(const nlohmann::json& j, vec4& vec)
	{
		vec.r = j.at("r").get<float>();
		vec.g = j.at("g").get<float>();
		vec.b = j.at("b").get<float>();
		vec.a = j.at("a").get<float>();
	}

	//setup only for 1 parameter matrices.
	void to_json(nlohmann::json& j, const mat4& mat)
	{
		j = { { "matrix", mat[0][0]} };
	}

	void from_json(const nlohmann::json& j, mat4& mat)
	{
		mat = mat4(j.at("matrix").get<float>());
	}
	#pragma endregion
}

namespace Engine
{
	bool Serializer::tryDeserializeScene(Scene& out, const std::string& sceneFile)
	{
		
		if (!Renderer::getInstance())
		{
			GE_CORE_FATAL("Failed to initialize opengl");
			return false;
		}

		auto data = ResourceManager::getInstance()->getJsonData(sceneFile);
		if (!(data.find("scene") != data.end()))
		{
			GE_CORE_FATAL("Unable to deserialize scene {0}", sceneFile);
			GE_CORE_FATAL("No scene found.");
			return false;
		}

		if (!(data["scene"].find("name") != data["scene"].end()))
		{
			GE_CORE_FATAL("Unable to deserialize scene {0}", sceneFile);
			GE_CORE_FATAL("Scene does not contain the \"name\" attribute.");
			return false;
		}

		out.m_name = data["scene"]["name"];

		//Loop through all entities and deserialize each
		for (const auto& item : data["scene"]["entities"])
		{
			if (!(item.find("tag") != item.end()))
			{
				GE_CORE_FATAL("Unable to deserialize scene {0}", sceneFile);
				GE_CORE_FATAL("An entity does not contain the \"tag\" attribute");
				return false;
			}

			Entity entity = out.createEntity(item["tag"]);
			
			if (!tryDeserializeEntity(entity, item, out))
			{
				
				GE_CORE_FATAL("Unable to deserialize scene {0}", sceneFile);
				GE_CORE_FATAL("The entity {0} has failed to serialize", item["tag"]);
				return false;
			
			}
		}
		
		return true;
	}

	std::string Serializer::serializeScene(Scene* scene, const std::string& sceneFile)
	{
		nlohmann::json sceneJson;
		nlohmann::json entitiesJson;
		scene->m_registry.each([&](entt::entity entityHandle)
			{
				Entity entity = Entity{ entityHandle, scene };
				if (!entity) return;

				if (entity.hasComponent<SerializableComponent>()) //skip entities that were generated/don't have this component
				{
					entitiesJson.push_back(serializeEntity(entity, sceneFile));
				}
			});

		sceneJson["scene"]["entities"] = entitiesJson;
		sceneJson["scene"]["name"] = scene->m_name;

		std::cout << sceneJson << std::endl;
		ResourceManager::getInstance()->saveJsonFile(sceneJson, sceneFile, "bda");
		return sceneJson.dump();
		//TODO: Bind serialization to GUI event once we have one.

	}

	nlohmann::json Serializer::serializeEntity(Entity& entity, const std::string& sceneFile)
	{


		if (!entity.hasComponent<TagComponent>())
		{
			GE_CORE_ERROR("An entity was created without a tag component and cannot be serialized.");
		}

		nlohmann::json components = nlohmann::json::array();

		if (entity.hasComponent<CameraComponent>())
		{
			auto c = entity.getComponent<CameraComponent>();
			nlohmann::json j;
			j["name"] = parseComponentToString(CO_CameraComponent);
			j["aspectRatio"] = c.aspectRatio;
			j["frustumWidth"] = c.frustumWidth;
			j["farZ"] = c.farZ;
			j["nearZ"] = c.nearZ;

			components.push_back(j);
		}

		if (entity.hasComponent<TransformComponent>())
		{
			auto c = entity.getComponent<TransformComponent>();
			nlohmann::json j;
			j["name"] = parseComponentToString(CO_TransformComponent);
			j["position"] = c.position;
			j["scale"] = c.scale;
			j["rotation"] = c.rotation;

			components.push_back(j);
		}

		if (entity.hasComponent<TextComponent>())
		{
			auto c = entity.getComponent<TextComponent>();
			nlohmann::json j;
			j["name"] = parseComponentToString(CO_TextComponent);
			j["text"] = c.text;

			components.push_back(j);
		}

		if (entity.hasComponent<MaterialComponent>())
		{
			auto c = entity.getComponent<MaterialComponent>();
			nlohmann::json j;
			j["name"] = parseComponentToString(CO_MaterialComponent);
			j["color"] = c.color;
			j["texName"] = c.texName;

			components.push_back(j);
		}

		if (entity.hasComponent<AnimationComponent>())
		{
			auto c = entity.getComponent<AnimationComponent>();
			nlohmann::json j;
			j["name"] = parseComponentToString(CO_AnimationComponent);
			j["numPerRow"] = c.numPerRow;
			j["frameRate"] = c.frameRate;
			j["texWidthFraction"] = c.texWidthFraction;
			j["texHeightFraction"] = c.texHeightFraction;
			j["texName"] = c.texName;
			j["numSprites"] = c.spritesOnSheet;

			components.push_back(j);
		}

		if (entity.hasComponent<VerticesComponent>())
		{
			auto c = entity.getComponent<VerticesComponent>();
			nlohmann::json j;
			j["name"] = parseComponentToString(CO_VerticesComponent);
			j["type"] = c.isSprite ? "sprite" : "polygon";

			//TODO: Serialize vertex if polygon once we know what that looks like.	

			components.push_back(j);
		}

		if (entity.hasComponent<ScriptComponent>())
		{
			auto c = entity.getComponent<ScriptComponent>();
			nlohmann::json j;
			j["name"] = parseComponentToString(CO_ScriptComponent);
			j["scriptName"] = c.m_instance->getScriptName();

			components.push_back(j);
		}

		if (entity.hasComponent<PhysicsComponent>())
		{
			auto c = entity.getComponent<PhysicsComponent>();
			nlohmann::json j;
			j["name"] = parseComponentToString(CO_PhysicsComponent);
			j["dimensions"] = c.boundingBox->getDimensions();
			j["position"] = c.boundingBox->getPosition();
		}

		//add all components and tag to json
		nlohmann::json entityJson;
		entityJson["components"] = components;
		entityJson["tag"] = entity.getComponent<TagComponent>().tag;

		return entityJson;
	}

	bool Serializer::tryDeserializeEntity(Entity& out, const nlohmann::json& entity, Scene& scene)
	{

		//Loop through all components and deserialize each
		for (const auto& component : entity["components"])
		{
			if (!(component.find("name") != component.end()))
			{
				GE_CORE_FATAL("Unable to deserialize entity {0}", entity["tag"]);
				GE_CORE_FATAL("A component does not contain the \"name\" attribute");
				return false;
			}

			auto name = parseComponent(component["name"].get<std::string>());

			//check each component type individually
			switch (name)
			{
			case CO_CameraComponent:
			{
				auto frustumWidth = component["frustumWidth"].get<float>();
				auto aspectRatio = component["aspectRatio"].get<float>();
				auto farZ = component["farZ"].get<float>();
				auto nearZ = component["nearZ"].get<float>();
				
				out.addComponent<CameraComponent>(frustumWidth, aspectRatio, farZ, nearZ);
				break;
			}
			case CO_TransformComponent:
			{
				auto position = component["position"].get<glm::vec3>();
				auto scale = component["scale"].get<glm::vec3>();
				auto rotation = component["rotation"].get<glm::vec3>();

				out.addComponent<TransformComponent>(position, scale, rotation);
				break;
			}
			case CO_MaterialComponent:
			{
				
				std::string texture = component["texName"];
				auto image = ResourceManager::getInstance()->getTexture(texture);
				
				//glm::vec4 color, GLuint texID, std::string texName, GLuint shaderID)
				out.addComponent <MaterialComponent>(component["color"].get<glm::vec4>(), image.texID, texture, 0); //0 Would be shaderID. waiting until shader code is imported ********
				break;
			}
			case CO_TextComponent:
			{
				std::string text = component["text"];

				out.addComponent<TextComponent>(text);
				break;
			}
			case CO_VerticesComponent:
			{
				//Check if vertices is of type sprite
				if (!(component.find("type") != component.end()))
				{
					GE_CORE_FATAL("Unable to deserialize entity {0}", entity["tag"]);
					GE_CORE_FATAL("A VerticesComponent does not contain the \"type\" attribute");
					return false;
				}

				if (component["type"] == "sprite")
				{
					out.addComponent<VerticesComponent>(GeometryFactory::getInstance()->getVerticesComponent(GeometryFactory::RT_Sprite));
				}

				break;
			}
			case CO_AnimationComponent:
			{
				std::string texture = component["texName"];
				auto spritesheet = ResourceManager::getInstance()->getSpritesheet(texture);

				out.addComponent<AnimationComponent>(spritesheet.texID, 0, spritesheet.texWidthFraction,
					spritesheet.texHeightFraction, spritesheet.spritesPerRow, spritesheet.numSprites);
				break;
			}
			case CO_ScriptComponent:
			{
				std::string scriptName = component["scriptName"];
				ScriptSerializer::linkAndDeserializeScript(out, scriptName);
				break;
			}
			case CO_PhysicsComponent:
			{
				glm::vec3 dimensions = component["dimensions"].get<glm::vec3>();
				glm::vec3 position = component["position"].get<glm::vec3>();

				out.addComponent<PhysicsComponent>(dimensions, position);
				break;
			}
			default:
			{
				GE_CORE_WARN("Could not deserialize component {0}", component["name"]);
				break;
			}
			}
		}

		out.addComponent<SerializableComponent>();

		return true;
	}
}