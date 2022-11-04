#include "Serializer.h"

#include "Engine/Entity.h"
#include "Engine/Log.h"
#include "Engine/ResourceManager.h"

namespace glm
{
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
}

namespace Engine
{
	bool Serializer::tryDeserializeScene(Scene& out, const std::string& sceneFile)
	{
		if (!out.initializeGL())
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

	void Serializer::serializeScene(const Scene& scene, const std::string& sceneFile)
	{
		//TODO: Implement serialization once we have more GUI stuff
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
				auto fov = component["fov"].get<float>();
				auto projection = component["projection"].get<glm::mat4>();
				auto viewport = component["viewport"].get<glm::vec2>();
				auto farZ = component["farZ"].get<float>();
				auto nearZ = component["nearZ"].get<float>();

				out.addComponent<CameraComponent>(fov, projection, viewport, farZ, nearZ);
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
			case CO_ColorComponent:
			{
				out.addComponent<ColorComponent>(component["color"].get<glm::vec4>());
				break;
			}
			case CO_TextureComponent:
			{
				std::string texture = component["texName"];
				auto image = ResourceManager::getInstance()->getTexture(texture);

				out.addComponent<TextureComponent>(image.texID, texture);
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
					out.addComponent<VerticesComponent>(scene.createSprite());
				}

				break;
			}
			case CO_AnimationComponent:
			{
				std::string texture = component["texName"];
				auto spritesheet = ResourceManager::getInstance()->getSpritesheet(texture);

				out.addComponent<AnimationComponent>(spritesheet.texID, 0, spritesheet.texWidthFraction, spritesheet.texHeightFraction, spritesheet.spritesPerRow);
				break;
			}
			default:
			{
				GE_CORE_WARN("Could not deserialize component {0}", component["name"]);
				break;
			}
			}
		}

		return true;
	}
}