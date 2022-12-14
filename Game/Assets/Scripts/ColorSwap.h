#include "Engine/Scripts/ScriptableBehavior.h"
#include "Engine/SceneBuilder/Components.h"

namespace Engine
{
	class ColorSwap : public Engine::ScriptableBehavior
	{
	public:
		std::map<int, glm::vec4> colors = {
			{0, glm::vec4(0.05f, 0.64f, 0.89f, 0.5f)}, // blue
			{1, glm::vec4(0.90f, 0.13f, 0.36f, 0.5f)}, // red
			{2, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)}, // green
			{3, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)}, // yellow
			{4, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)}, // purple
			{5, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)}	// cyan
		};
		// Difficulty indicating the number of colors the tile will swap between
		int difficultyLevel = 1;
		int currentColorIndex;

		void swapMyColor();
		void checkLevelComplete();
	protected:
		virtual void onCreate() override;
		virtual void onUpdate(const Engine::DeltaTime& dt) override;

	public:
		virtual std::string getScriptName() override { return "ColorSwap"; }
	};
}