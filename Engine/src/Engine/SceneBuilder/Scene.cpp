#include "glad/glad.h"
#include "Engine/SceneBuilder/Scene.h"

#include <glm/gtc/type_ptr.hpp>

#include "Engine/Utilities/Log.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "Entity.h"
#include <Engine/ResourceManagement/ShaderGenerator.h>
#include "Components.h"
#include <Engine/Utilities/DeltaTime.h>
#include "Engine/Scripts/ScriptableBehavior.h"
#include "Engine/ResourceManagement/ResourceManager.h"
#include "InputSystem.h"
#include "Engine/ResourceManagement/Serializer.h"

namespace Engine
{
/*
Issues:
- Sprite size is inversely porpotional to the window size (shrinking window expands sprite)
*/

#pragma region Runtime Functions
	void Scene::onRuntimeStart()
	{
		//initialize the window for UI
		/*if (showUI) {
			if (!initializeUI()) return;
		}*/

		glEnable(GL_DEPTH_TEST);
		// framebuffer configuration
		// -------------------------
		glGenFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		// create a color attachment texture
		glGenTextures(1, &textureColorbuffer);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1920, 1080, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
		// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
		unsigned int rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1920, 1080); // use a single renderbuffer object for both a depth AND stencil buffer.
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
		// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now

		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
	// positions   // texCoords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f, 1.0f
		};

		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));


		ImGui::CreateContext();
		ImGui_ImplOpenGL3_Init("#version 330");
		ImGui_ImplGlfw_InitForOpenGL(m_window, true);

		createEntities();
		//InputSystem::getInstance()->init(m_window);

		while (!glfwWindowShouldClose(m_window))
		{
			m_deltaTime.updateDeltaTime();
			onRuntimeUpdate(m_deltaTime);
		}

		onRuntimeStop();
	}

	void Scene::onRuntimeStop()
	{
		glfwTerminate();
	
		const auto view = getEntities<const VerticesComponent>();
		for (auto [entity, vertices] : view.each())
		{
			glDeleteVertexArrays(1, &vertices.vaoID);
			glDeleteBuffers(1, &vertices.vboID);
		}
		glDeleteProgram(m_programId);
	}

	void Scene::onRuntimeUpdate(const DeltaTime& dt)
	{	
		//get a view on entities with a script Component, and execute their onUpdate.
		const auto entities = getEntities<ScriptComponent>();
		for (auto [entity, script] : entities.each())
		{
			if (script.m_instance->m_enabled) script.m_instance->onUpdate(dt);//don't update if entity is disabled
		}		
		
		//We have two distinct windows to manage now - one for the main scene and the UI window
		//For each window we change the GL context, render to the window, then swap buffers


		//Main window
		// //glfwMakeContextCurrent(m_window);
		//
		// glGenFramebuffers(1, &m_fbo);
		//
		// glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		/*glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 1920, 1080, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_fbo, 0);*/
		
		// unsigned int textureColorbuffer;
		// glGenTextures(1, &textureColorbuffer);
		// glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1920, 1080, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// glBindTexture(GL_TEXTURE_2D, 0);
		//
		// // attach it to currently bound framebuffer object
		// glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
		//
		// GLuint rbo;
		//
		// glGenRenderbuffers(1, &rbo);
		// glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		// glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1920, 1080);
		// glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		/*if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);


			glDeleteFramebuffers(1, &fbo);
		}*/

		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
		glEnable(GL_DEPTH_TEST);

		glUseProgram(m_programId);

		renderScene(dt);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(m_programId2);
		glBindVertexArray(quadVAO);
		glDisable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//glfwSwapBuffers(m_window);
		//glfwPollEvents();



		//need to create a frame buffer object and add a texture of the rendered game to it
		

		

		//UI window
		if (showUI) {
			glfwMakeContextCurrent(m_window);
			renderUI();
			glfwSwapBuffers(m_window);
		}

		//Execute onLateUpdate().
		for (auto [entity, script] : entities.each())
		{
			if (script.m_instance->m_enabled) script.m_instance->onLateUpdate(dt);//don't update if entity is disabled
		}

		glfwPollEvents();
	}
#pragma endregion

#pragma region OpenGL Scene management
		
	//Insitialize OpenGL, returning true if successful. Window based on GLFW.
	bool Scene::initializeGL()
    {
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "BlueDemise", nullptr, nullptr); //switch to unique ptr with deleter for RAII?
		if (m_window == nullptr)
		{
			GE_CORE_ERROR("Failed to create GLFW window");
			glfwTerminate();
			return false;
		}

		//if (showUI) {
		//	m_UIwindow = glfwCreateWindow(m_windowWidth, m_windowHeight, "User Interface", nullptr, nullptr); //switch to unique ptr with deleter for RAII?
		//	if (m_UIwindow == nullptr)
		//	{
		//		GE_CORE_ERROR("Failed to create GLFW window (UI)");
		//		glfwTerminate();
		//		return false;
		//	}
		//}

		glfwMakeContextCurrent(m_window);

		//Setup a callback to update the viewport size when the window is resized
		//glfwSetWindowUserPointer(m_window, reinterpret_cast<void*>(this));
		//glfwSetWindowSizeCallback(m_window, windowResizeCallback);

		//Setting the icon
		ResourceManager::getInstance()->setAppIcon(*m_window);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			GE_CORE_ERROR("Failed to initialize GLAD");
			glfwTerminate();
			return false;
		}

		loadShaders();
		m_programId2 = loadShaders2();

		glfwSwapInterval(1);
		glClearColor(0.1f, 0.1f, 0.1f, 1);

		return true;
    }

	bool Scene::initializeUI()
	{
		//Initialize the UI using ImGui OpenGL v3.3
		/*if (!UserInterface::initialize(m_window))
		{
			return false;
		}*/

		//Load custom fonts - we must load a different for each size we require
		//We are using MyriadPro currently as the Freedom font does not display numeric values
		if (!UserInterface::loadFont("Assets/MyriadPro_bold.otf", 12, "Freedom_12"))
		{
			//Error loading font
		}

		if (!UserInterface::loadFont("Assets/MyriadPro_bold.otf", 18, "Freedom_18"))
		{
			//Error loading font
		}

		//if (!UserInterface::loadFont("Assets/MyriadPro.otf", 12, "MyriadPro"))
		//{
		//	//Error loading font
		//}

		const int menuHeight = 18;
		const auto panelWidth = 0.2f * m_windowWidth;
		const auto halfWindowHeight = 0.5f * (m_windowHeight - menuHeight);

		m_tagDialog.setPosition(glm::uvec2(0.5f * m_windowWidth, 0.5f * m_windowHeight));

		m_explorerPanel.setPosition(glm::uvec2(0, menuHeight));
		m_explorerPanel.setDimension(glm::uvec2(panelWidth, halfWindowHeight));

		m_hierarchyPanel.setPosition(glm::uvec2(panelWidth - 1, menuHeight));
		m_hierarchyPanel.setDimension(glm::uvec2(panelWidth, halfWindowHeight));

		m_inspectorPanel.setPosition(glm::uvec2(panelWidth * 2 - 2, menuHeight));
		m_inspectorPanel.setDimension(glm::uvec2(panelWidth, halfWindowHeight));

		m_componentsPanels[Components].setPosition(glm::uvec2(panelWidth * 3 - 3, menuHeight));
		m_componentsPanels[Components].setDimension(glm::uvec2(panelWidth, halfWindowHeight));

		//TODO - Does this need to be read from a .json file?
		m_componentsPanels[Components].addComponent("Transform");
		m_componentsPanels[Components].addComponent("Camera");
		m_componentsPanels[Components].addComponent("Color");
		m_componentsPanels[Components].addComponent("Animation");
		m_componentsPanels[Components].addComponent("Texture");

		m_componentsPanels[Attributes].setPosition(glm::uvec2(panelWidth * 4 - 4, menuHeight));
		m_componentsPanels[Attributes].setDimension(glm::uvec2(panelWidth, halfWindowHeight));

		m_componentsPanels[ActiveComponents].setPosition(glm::uvec2(panelWidth * 4 - 4, menuHeight + 0.5f * (m_windowHeight - menuHeight)));
		m_componentsPanels[ActiveComponents].setDimension(glm::uvec2(panelWidth, halfWindowHeight));

		m_inspectorPanel.setRegistry(&m_registry);

		return true;

	}

	void Scene::shutdownUI()
	{
		UserInterface::shutdown();
	}

	//Callback to update window size when it changes
	//TODO: Handle screen resizing
	void windowResizeCallback(GLFWwindow* window, int width, int height)
	{
		/*Scene* scene = reinterpret_cast<Scene*>(glfwGetWindowUserPointer(window));
		auto cameraView = scene->getEntities<const CameraComponent>();
		auto &camera = scene->m_registry.get<CameraComponent>(cameraView.back());
		camera.viewport.x = width;
		camera.viewport.y = height;*/
	}

	

	//clears the window and renders all entities that need to be rendered (those with transform, vertices, color).
	void Scene::renderScene(const DeltaTime& dt)
	{
		glClear(GL_COLOR_BUFFER_BIT);

		auto cameraView = getEntities<const CameraComponent>();
		const auto camera = m_registry.get<CameraComponent>(cameraView.back());
		glm::mat4 pm = glm::ortho(0.f, camera.viewport.x, camera.viewport.y, 0.f, camera.nearZ, camera.farZ);
		glm::mat4 vm = glm::translate(glm::mat4(1.f), glm::vec3(0, 0, -10.f)); //position of camera in world-space


		//Render all entities
		//Get entities that contain transform & vertices & color components,
		const auto solidObj = getEntities<const TransformComponent, const VerticesComponent, const ColorComponent>();

		//For each updatable entity (with transform, vertices, and color components), draw them.
		for (auto [entity, transform, vertices, color] : solidObj.each())
		{
			//Bind Texture
			if (m_registry.all_of<TextureComponent>(entity))
			{
				const auto texture = m_registry.get<const TextureComponent>(entity);
				glBindTexture(GL_TEXTURE_2D, texture.texID);
			}

			if (m_registry.all_of<AnimationComponent>(entity))
			{
				glBindBuffer(GL_ARRAY_BUFFER, vertices.vboID);
				auto& anim = m_registry.get<AnimationComponent>(entity);
				anim.deltaTime += dt;

				if (anim.deltaTime > anim.animationSpeed)
				{
					anim.deltaTime = 0;
					anim.currentIndex++;
					if (anim.currentIndex > anim.spritesOnSheet) anim.currentIndex = 0;
				}

				//Calculation for finding the sprite in a texture.
				const float tx = (anim.currentIndex % anim.numPerRow) * anim.texWidthFraction;
				const float ty = (anim.currentIndex / anim.numPerRow + 1) * anim.texHeightFraction;

				//bind VBO
				float vertices[] =
				{
					// positions  // texture coords (UV coords)
					0.f, 0.f, 0.f,  tx, ty,														// top left
					1.f, 0.f, 0.f,  tx + anim.texWidthFraction, ty,								// top right
					1.f, 1.f, 0.f,  tx + anim.texWidthFraction, ty + anim.texHeightFraction,	// bottom right
					0.f, 1.f, 0.f,  tx, ty + anim.texHeightFraction								// bottom left
				};

				//Buffer new data into VBO
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

				glBindTexture(GL_TEXTURE_2D, anim.texID);
			}
			else 
			{
				glBindBuffer(GL_ARRAY_BUFFER, vertices.vboID);
				float vertices[] =
				{
					// positions  // texture coords (UV coords)

					0.f, 0.f, 0.f,  0.f, 0.f,  // top left
					1.f, 0.f, 0.f,  1.f, 0.f,  // top right
					1.f, 1.f, 0.f,  1.f, 1.f,  // bottom right
					0.f, 1.f, 0.f,  0.f, 1.f,  // bottom left
				};

				//Buffer new data into VBO
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
			}

			//Update the MVP
			const glm::mat4 mvp = updateMVP(transform, vm, pm);

			//Set the color of the object
			setColor(mvp, color.color);
			
			glBindVertexArray(vertices.vaoID);

			glDrawElements(GL_TRIANGLES, vertices.numIndices, GL_UNSIGNED_INT, nullptr);
		}
	}

	//Update an MVP matrix, with the MVP generated in the function and returned.
	glm::mat4 Scene::updateMVP(TransformComponent transform, glm::mat4 view, glm::mat4 projection)
	{
		//Setup model view matrix
		glm::mat4 mvm = glm::mat4(1.f);
		mvm = glm::translate(mvm, transform.position);
		mvm = glm::rotate(mvm, transform.rotation.x, glm::vec3(1, 0, 0));
		mvm = glm::rotate(mvm, transform.rotation.y, glm::vec3(0, 1, 0));
		mvm = glm::rotate(mvm, transform.rotation.z, glm::vec3(0, 0, 1));
		mvm = glm::scale(mvm, transform.scale);

		//Calculate MVP
		glm::mat4 mvp = projection * view * mvm;
	
		return mvp;
	}

	void Scene::renderUI()
	{
		//This is required to make sure the UI window framebuffer is cleared
		glClear(GL_COLOR_BUFFER_BIT);

		//UserInterface::startUI();


		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		ImGui::Begin("Game");
		{
			ImGui::BeginChild("GameRenderer");
			ImVec2 wsize = ImGui::GetWindowSize();
			ImGui::Image((ImTextureID)m_fbo, ImVec2(1920,1080), ImVec2(0, 1), ImVec2(1, 0));
			ImGui::EndChild();
		}
		
		m_tagDialog.update();
		m_tagDialog.show();

		ImGui::End();
		
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		

		/*m_mainMenu.show();

		m_explorerPanel.show();
		m_hierarchyPanel.show();
		m_inspectorPanel.show();

		for (auto& panel : m_componentsPanels)
		{
			panel.show();
		}*/


		//UserInterface::endUI();

		//UI handlers===========================================================

		if (m_mainMenu.getIsSaveItemClicked())
		{
			//Save entities and components to a JSON file
			Engine::Serializer::serializeScene(this, "testScene");
		}

		//We look inside our own local entity map to search for
		//the entity the user clicked on in the Inspector panel 
		m_inspectorPanel.setSelectedEntity(m_entityHandles[m_hierarchyPanel.getSelectedEntity()]);

		if (m_hierarchyPanel.isAddButtonClicked())
		{
			m_tagDialog.setIsVisible(true);
		}

		//If we click on OKAY when labelling the new entity and no name has been set it will be called ENTITY
		//Otherwise we label it based on the name input in the tag dialog box
		if (m_tagDialog.getButtonState().okay)
		{
			auto tag = m_tagDialog.getTag();

			if (tag.empty())
			{
				tag = "<Entity_" + std::to_string(Entity::getTotalEntities() + 1) + ">";
			}

			Entity entity = createEntity(tag);
		}

		if (m_tagDialog.getButtonState().cancel)
		{
			//Do nothing if we cancel
		}

		if (m_componentsPanels[2].isAddButtonClicked())
		{
			//Check which component was selected to be added to entity
			auto component = m_componentsPanels[2].getSelectedComponent();

			//Get the handle on the selected entity to add the component to
			auto handle = m_entityHandles[m_hierarchyPanel.getSelectedEntity()];

			if (component == "Camera")
			{
				auto camera = m_registry.any_of<CameraComponent>(handle);

				//Only add this component if there isn't already one attached
				if (!camera)
				{
					m_registry.emplace<CameraComponent>(
						handle,
						90.f,
						glm::mat4(1.f),
						glm::vec2(4.8, 4.8),
						100.0f,
						0.1f
						);
				}
			}

			if (component == "Transform")
			{
				auto transform = m_registry.any_of<TransformComponent>(handle);

				//Only add this component if there isn't already one attached
				if (!transform)
				{
					m_registry.emplace<TransformComponent>(
						handle,
						glm::vec3(0, 0, 0),
						glm::vec3(1, 1, 1),
						glm::vec3(0, 0, 0));
				}
			}

			if (component == "Color")
			{
				auto color = m_registry.any_of<ColorComponent>(handle);

				//Only add this component if there isn't already one attached
				if (!color)
				{
					m_registry.emplace<ColorComponent>(
						handle,
						glm::vec4(1, 1, 1, 1.0f));
				}
			}

			if (component == "Texture")
			{
				auto texture = m_registry.any_of<TextureComponent>(handle);

				//Only add this component if there isn't already one attached
				if (!texture)
				{
					m_registry.emplace<TextureComponent>(
						handle,
						0,
						"<Texture>");
				}
			}

			//if (component == "Animation")
			//{
			//	auto animation = m_registry.any_of<AnimationComponent>(handle);

			//	//Only add this component if there isn't already one attached
			//	if (!animation)
			//	{
			//		m_registry.emplace<AnimationComponent>(
			//			handle,
			//			0,
			//			1.0f,
			//			1.0f,
			//			1.0f,
			//			1);
			//	}
			//}
		}
	}

	//loads and generates shaders to be used in scene. Replace with shader wrappers as per the .h todo.
	void Scene::loadShaders()
	{
		
		std::string vertexData = ResourceManager::getInstance()->getShaderData("Fill.vs");
		std::string fragmentData = ResourceManager::getInstance()->getShaderData("Fill.fs");

		ShaderGenerator shaderGenerator(vertexData.c_str(), fragmentData.c_str());
		
		m_programId = shaderGenerator.getProgramId();
		glUseProgram(m_programId);
	}

	GLuint Scene::loadShaders2()
	{

		std::string vertexData = ResourceManager::getInstance()->getShaderData("framebuffer.vs");
		std::string fragmentData = ResourceManager::getInstance()->getShaderData("framebuffer.fs");

		ShaderGenerator shaderGenerator(vertexData.c_str(), fragmentData.c_str());

		return shaderGenerator.getProgramId();
	}
#pragma endregion
	
#pragma region Entity Creation
	//Create an entity from the m_registry with the provided tag component, and return the entity.
	Entity Scene::createEntity(std::string tag)
	{
		Entity entity(m_registry.create(), this);
		entity.addComponent<TagComponent>(tag);
		m_entityHandles[tag] = entity.getHandle();
		m_hierarchyPanel.addEntity(tag);
		return entity;
	}

	// Creates entities that are to be used in the scene. Replace with serialized entities as per the .h todo.
	void Scene::createEntities()
    {
		ResourceManager::ImageData image = ResourceManager::getInstance()->getTexture("SpriteSheet.png");
		ResourceManager::SpriteSheet spriteSheet = ResourceManager::getInstance()->getSpritesheet("SpriteSheet.png");

		Entity triangle2 = createEntity("triangle2");
		triangle2.addComponent<TransformComponent>(
			glm::vec3(0.f, 0, 0),
			glm::vec3(image.height, image.width, 1),
			glm::vec3(0, 0, 0)
			);
		triangle2.addComponent<VerticesComponent>(createSprite());
		triangle2.addComponent<AnimationComponent>(spriteSheet.texID, 0.f, spriteSheet.texWidthFraction, spriteSheet.texHeightFraction, spriteSheet.spritesPerRow, spriteSheet.numSprites);
		triangle2.addComponent<ColorComponent>(glm::vec4(1, 1, 1, 1));

		Entity rectangle = createEntity("rect");
		rectangle.addComponent<TransformComponent>(
			glm::vec3(350, 350, 0),
			glm::vec3(100, 100, 1),
			glm::vec3(0, 0, 0)
			);
		rectangle.addComponent<VerticesComponent>(createRectangle());
		rectangle.addComponent<ColorComponent>(glm::vec4(1, 0, 0, 1));

		//TODO: After Serialization: Bind Entities HERE ***
		const auto scriptEntities = getEntities<ScriptComponent>();
		for (auto& [entity, script] : scriptEntities.each())
		{
			if (!script.m_instance)
			{
				script.m_instance = script.instantiateScript();
				script.m_instance->m_entity = Entity{ entity, this };
				script.m_instance->onCreate();
			}
		}
    }
#pragma endregion

#pragma region Renderable Entities
	
	//Return the VBO for sprites. If it doesn't exist, create it.
	GLuint Scene::getVBO(RenderableType type) 
	{
		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		if (type == RT_Sprite)
		{
			float vertices[] = 
			{
				// positions  // texture coords (UV coords)
				0.f, 0.f, 0.f,  0.f, 0.f,  // top left
				1.f, 0.f, 0.f,  1.f, 0.f,  // top right
				1.f, 1.f, 0.f,  1.f, 1.f,  // bottom right
				0.f, 1.f, 0.f,  0.f, 1.f,  // bottom left
			};
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
		}
		else if (type == RT_Rectangle)
		{
			float vertices[] = 
			{
				// positions 
				0.f, 0.f, 0.f,  // top left
				1.f, 0.f, 0.f,  // top right
				1.f, 1.f, 0.f,  // bottom right
				0.f, 1.f, 0.f   // bottom left
			};
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
		}
		else 
		{
			GE_CORE_ERROR("A vbo was declared with incorrect vertex data.");
		}
		return vbo;
	}

	//Return the VAO for sprites. If it doesn't exist, create it.
	GLuint Scene::getVAO()
	{
		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		return vao;
	}

	//Return the IBO for sprites. If it doesn't exist, create it.
	GLuint Scene::getIBO(RenderableType type)
	{
		GLuint ibo;
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

		if (type == RT_Rectangle) {
			unsigned int indices[6] =
			{
				0, 1, 2,  //first triangle
				2, 3, 0,  //second triangle
			};
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		}
		else 
		{
			GE_CORE_ERROR("An ibo was declared with incorrect index data...");
		}
		return ibo;
	}

	//Set the color of the current drawable object. This would need to be run per entity/renderable.
	void Scene::setColor(glm::mat4 mvp, glm::vec4 color) 
	{
		GLuint colorUniformID = glGetUniformLocation(m_programId, "col");
		GLuint mvpID = glGetUniformLocation(m_programId, "mvp");

		//Sets color of shader
		glUniform4fv(colorUniformID, 1, glm::value_ptr(color));
		glUniformMatrix4fv(mvpID, 1, GL_FALSE, glm::value_ptr(mvp));
	}

	//Placeholder functio, can be replaced by serialized objects.
	VerticesComponent Scene::createSprite()
	{
		VerticesComponent vc;
		vc.vertexAttributes.push_back(VertexAttribute(0, 3, GL_FLOAT, GL_FALSE, 0));
		vc.vertexAttributes.push_back(VertexAttribute(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 3));
		vc.stride = sizeof(float) * m_quadTexCoordinates;
		vc.numIndices = m_quadIndices;

		vc.vaoID = getVAO();
		vc.vboID = getVBO();
		vc.iboID = getIBO();

		setupVertexAttribPtr(vc);

		return vc;
	}

	//Placeholder function, can be replaced by serialized objects.
	VerticesComponent Scene::createRectangle()
	{
		VerticesComponent vc;
		vc.vertexAttributes.push_back(VertexAttribute(0, 3, GL_FLOAT, GL_FALSE, 0));
		vc.stride = sizeof(float) * m_quadTexCoordinates;
		vc.numIndices = m_quadIndices;

		vc.vaoID = getVAO();
		vc.vboID = getVBO(RT_Rectangle);
		vc.iboID = getIBO();

		setupVertexAttribPtr(vc);

		return vc;
	}

	//Define vertex attributes
	void Scene::setupVertexAttribPtr(VerticesComponent& vc)
	{
		for (int i = 0; i < vc.vertexAttributes.size(); i++)
		{
			const auto attribute = vc.vertexAttributes[i];
			glEnableVertexAttribArray(i);
			glVertexAttribPointer(attribute.index, attribute.size, attribute.type, attribute.normalized, vc.stride, (const void*)attribute.pointer);
		}

		glBindVertexArray(0);
	}
	
#pragma endregion
	
}
