#include <Engine/ResourceManagement/ShaderNorms.h>
#include <Engine/ResourceManagement/ShaderGenerator.h>
#include <Engine/ResourceManagement/AdvancedShaderDistributor.h>
#include <Engine/ResourceManagement/ResourceManager.h>
#include "glad/glad.h"

using namespace std;

namespace Engine
{
	ShaderNorms* ShaderNorms::m_singleton = nullptr;

	ShaderNorms::ShaderNorms()
	{
		addShader(ShaderFillType::DEFAULT_FILL_TYPE);
		addShader(ShaderFillType::FillType::FT_COLOR_FILL);
		addShader(ShaderFillType::FillType::FT_GRADIENT_FILL);
		addShader(ShaderFillType::FillType::FT_TEXT_FILL);
		
		//add advanced shaders
		for (int i = 0; i < m_advancedShaderNames.size(); i++)
		{
			addAdvancedShader(i, m_advancedShaderNames[i]);
		}
	}

	ShaderNorms::~ShaderNorms()
	{
		for (auto [key, value] : m_shaders)
		{
			glDeleteProgram(value.getProgramId());
		}
	}

	ShaderNorms* ShaderNorms::getInstance()
	{
		if (m_singleton == nullptr)
		{
			m_singleton = new ShaderNorms();
		}
		return m_singleton;
	}

	void ShaderNorms::setUniformValuef(int& bind, std::string uniformName, float value)
	{
		glUniform1f(glGetUniformLocation(m_advancedShaders.at(bind).getAdvancedProgramId(), uniformName.c_str()), value);
	}

	void ShaderNorms::setUniformValuei(int& bind, std::string uniformName, int value)
	{
		glUniform1i(glGetUniformLocation(m_advancedShaders.at(bind).getAdvancedProgramId(), uniformName.c_str()), value);
	}

	void ShaderNorms::setUniformValueb(int& bind, std::string uniformName, bool value)
	{
		glUniform1i(glGetUniformLocation(m_advancedShaders.at(bind).getAdvancedProgramId(), uniformName.c_str()),
		!value ? 0 : 1);
	}

	//tests the current renderable or vbo being rendered with the previous. If the current
	//stride doesn't match the previous, a new shader is loaded. Gets called by renderer.
	void ShaderNorms::update(double deltaTime, int advancedShaderBind, int stride,
		int& textureCoordinates, int& colorCoordinates, int& gradientCoordinates, GLuint& programId, bool firstPass)
	{
		if (firstPass) 
		{
			m_timeCount += deltaTime;
		}

		if (advancedShaderBind != m_currentAdvancedShaderBind)
		{
			m_currentAdvancedShaderBind = advancedShaderBind;
			if (m_currentAdvancedShaderBind != -1)
			{
				assignProgram(m_advancedShaders.at(m_currentAdvancedShaderBind).getAdvancedProgramId(), programId);

				//quick access since we don't have a way of determining if a shader has a uniform time variable
				if (advancedShaderBind == m_advancedShaderBinds.at("SinWave"))
				{
					//update time variables in real-time
					setUniformValuef(advancedShaderBind, "time", 2);
					setUniformValuef(advancedShaderBind, "timeCounter", m_timeCount);
				}
			}
			else
			{
				//reset m_currentStride
				assignsNewStride(stride);
				assignOnStride(stride, textureCoordinates, colorCoordinates, gradientCoordinates, programId);
			}
		}
		else if (m_currentAdvancedShaderBind == -1 && assignsNewStride(stride))
		{
			assignOnStride(stride, textureCoordinates, colorCoordinates, gradientCoordinates, programId);
		}
	}

	void ShaderNorms::assignOnStride(int stride, int& textureCoordinates, int& colorCoordinates,
		int& gradientCoordinates, GLuint& programId)
	{
		if (stride == textureCoordinates)
		{
			assignProgram(getShaderReference(), programId);
		}
		else if (stride == colorCoordinates)
		{
			assignProgram(getShaderReference(ShaderFillType::FillType::FT_COLOR_FILL), programId);
		}
		else if (stride == gradientCoordinates)
		{
			assignProgram(getShaderReference(ShaderFillType::FillType::FT_GRADIENT_FILL), programId);
		}
	}

	//true if a new stride amount is introduced so that GL doesn't reload the same shader
	bool ShaderNorms::assignsNewStride(int stride)
	{
		bool changed = false;
		if (stride != m_currentStride)
		{
			m_currentStride = stride;
			changed = true;
		}

		return changed;
	}

	void ShaderNorms::addShader(ShaderFillType::FillType shaderName)
	{
		string shaderStr = getShaderNameString(shaderName);
		m_shaders.insert(pair(shaderName, ShaderGenerator(
			ResourceManager::getInstance()->getShaderData(shaderStr + ".vs").c_str(),
			ResourceManager::getInstance()->getShaderData(shaderStr + ".fs").c_str()
		)));
		GE_CORE_INFO("[Shader Norms] " + getShaderNameString(shaderName) + " shader added.");
	}

	void ShaderNorms::addAdvancedShader(int bind, std::string& shaderName)
	{
		string asSource = ResourceManager::getInstance()->getShaderData(shaderName + ".as");
		m_advancedShaders.insert(pair(bind, AdvancedShaderDistributor(asSource,
			m_shaders.at(ShaderFillType::FillType::FT_TEXTURE_FILL))));
		m_advancedShaderBinds.insert(pair(shaderName, bind));
	}

	void ShaderNorms::resetTimeCount()
	{
		m_timeCount = 0;
	}

	GLuint ShaderNorms::getShaderReference(ShaderFillType::FillType shaderName)
	{
		return m_shaders.at(shaderName).getProgramId();
	}

	GLuint ShaderNorms::getAdvancedShaderReference(int bind)
	{
		return m_advancedShaders.at(bind).getAdvancedProgramId();
	}

	std::vector<std::string>& ShaderNorms::getShaderNames()
	{
		return m_advancedShaderNames;
	}

	void ShaderNorms::assignProgram(GLuint shaderBind, GLuint& shaderAccessor)
	{
		shaderAccessor = shaderBind;
		glUseProgram(shaderBind);
	}

	string ShaderNorms::getShaderNameString(ShaderFillType::FillType shaderName)
	{
		switch (shaderName)
		{
		case ShaderFillType::FillType::FT_TEXTURE_FILL:
			return "TextureFill";
		case ShaderFillType::FillType::FT_COLOR_FILL:
			return "ColorFill";
		case ShaderFillType::FillType::FT_GRADIENT_FILL:
			return "GradientFill";
		case ShaderFillType::FillType::FT_TEXT_FILL:
			return "TextFill";
			
		}
	}
}