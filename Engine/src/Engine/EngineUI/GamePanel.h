#pragma once
#include "UserInterface.h"

namespace Engine
{
	class Window;
	class GamePanel : public UserInterface
	{
	public:
		void setInitialPosition();
		void show(Window& window);
		glm::vec2 getPosition() { return m_position; };
	private:

	};
}