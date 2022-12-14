#pragma once
#include "Engine/Core.h"
#include <array>
#include "Engine/EngineUI/ComponentsPanel.h"
#include "Engine/EngineUI/ExplorerPanel.h"
#include "Engine/EngineUI/MainMenu.h"
#include "Engine/EngineUI/GamePanel.h"
#include "Engine/EngineUI/InspectorPanel.h"
#include "Engine/EngineUI/TagDialog.h"
#include "Engine/EngineUI/HierarchyPanel.h"


namespace Engine
{
    class Scene;
    class Window;

	class ENGINE_API UIRenderer
	{
	public:
        bool initializeUI(Window& window, Scene& scene);
        void shutdownUI();
        void renderUI(Scene& scene, Window& window);
        void updateHierarchyPanel(std::string tag, const Entity entity);
        GamePanel* getGamePanel() { return &m_gamePanel; }

        //Gets the current position of the Game Window
        glm::vec2 getGameWindowPos() { return m_gamePanel.getPosition(); }

	private:
        //component panel is the UI element that lists the components
        ComponentsPanel m_componentsPanel = "Components";

        //main menu is the UI element that shows the game
        MainMenu m_mainMenu;

        //explorer panel is the UI element that shows the file explorer
        ExplorerPanel m_explorerPanel;

        //entities panel is the UI element that lists all the entities
        HierarchyPanel m_hierarchyPanel;

        //inspector panel is the UI element that lists all the components for each entity
        InspectorPanel m_inspectorPanel;

        //game panel is the UI element that shows the render of the game
        GamePanel m_gamePanel;

        //dialog box to allow user to input the tag name for an entity
        TagDialog m_tagDialog;
	};
}