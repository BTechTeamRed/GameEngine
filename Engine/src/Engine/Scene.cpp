#include "Scene.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "Entity.h"
#include "ShaderGenerator.h"
#include "SourceGatherer.h"
#include "glad/glad.h"

namespace Engine
{
    Entity Scene::createEntity(std::string tag)
    {
        Entity entity(m_registry.create(), this);
        entity.addComponent<TagComponent>(tag);
        return entity;
    }

	void Scene::onRuntimeStart()
	{
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


		m_window = glfwCreateWindow(640, 480, "BlueDemise", nullptr, nullptr); //switch to unique ptr with deleter for RAII?
		if (m_window == nullptr)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return;
		}

		glfwMakeContextCurrent(m_window);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			return;
		}

		input::SourceGatherer sg(new std::string[2]{
			"..\\Engine\\src\\Engine\\Shaders\\Fill.vs",
			"..\\Engine\\src\\Engine\\Shaders\\Fill.fs"
			}, 2);

		/* -- Read source from cmd
		std::cout << "source(vs):" << std::endl << sg.getSource()
			<< "source(fs):" << std::endl << sg.getSource(1) << std::endl;
			*/

		input::ShaderGenerator shaderGenerator(sg.getSource().c_str(), sg.getSource(1).c_str());
		/* --Test program ID
		std::cout << "programID: " << shaderGenerator.getProgramId() << std::endl;
		*/

		glfwSwapInterval(1);
		glClearColor(0, 0, 0, 0); //set clear color to white
		glEnable(GL_DEPTH_TEST);
		//enable transparency
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		while (!glfwWindowShouldClose(m_window))
		{
			onRuntimeUpdate();
		}
	}

	void Scene::onRuntimeStop()
	{
		glfwTerminate();
	}

	void Scene::onRuntimeUpdate()
	{
		renderScene();
		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}

	void Scene::renderScene()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 mvp;
		glm::mat4 mvm;
		glm::mat4 pm;

		auto view = getEntities<const TransformComponent, const VerticesComponent, const ColorComponent>();
		auto cameraView = getEntities<const TransformComponent, const CameraComponent>();
		const auto camera = m_registry.get<CameraComponent>(cameraView.back());//Gets the last entity with these components, if there are multiple (TODO: Switch to wrapper)
		pm = glm::ortho(0.0f, camera.viewport.x, 0.0f, camera.viewport.y, camera.nearZ, camera.farZ);
		for (auto [entity, transform, vertices, color] : view.each())
		{

			//Setup mvp and mvm matrix
			mvm = glm::mat4(1.f);
			mvm = glm::translate(mvm, transform.position);
			mvm = glm::rotate(mvm, transform.rotation.x, glm::vec3(1, 0, 0));
			mvm = glm::rotate(mvm, transform.rotation.y, glm::vec3(0, 1, 0));
			mvm = glm::rotate(mvm, transform.rotation.z, glm::vec3(0, 0, 1));
			mvm = glm::scale(mvm, transform.scale);

			mvp = pm * mvm;

			//glUseProgram(programID)
			//glUniform4fv(colorUniformID, 1, (const float *)glm::value_ptr(color.color)); //Consider changing the way we cast here, probably not best practices
			//glUniformMatrix4fv(projMatID, 1, (const float *)glm::value_ptr(pm))

			glBindVertexArray(vertices.vaoID);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertices.iboID);
			glDrawElements(GL_TRIANGLES, vertices.numIndices, GL_UNSIGNED_INT, 0);
		}
	}
}