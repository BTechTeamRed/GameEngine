#pragma once
#include "UserInterface.h"

namespace Engine
{
	class Window;
	class GamePanel : public UserInterface
	{
	public:

		virtual ~GamePanel() {}

		void show(Window& window);

		glm::vec2 getPosition() { return m_position; };

		const int TITLE_BAR_HEIGHT = 36;

	private:

	};
}