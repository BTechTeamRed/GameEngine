#include "Engine/EngineUI/ComponentsPanel.h"

namespace Engine
{
	ComponentsPanel::ComponentsPanel(const std::string& name) : m_name(name) {}

	bool ComponentsPanel::isAddButtonClicked() const
	{
		return m_isAddButtonClicked;
	}

	const std::string& ComponentsPanel::getSelectedComponent() const
	{
		return m_selectedComponent;
	}

	void ComponentsPanel::addComponent(const std::string& component)
	{
		m_components.push_back(component);
	}

	//struct for radio buttons for components
	struct ComponentsButton 
	{
		bool state; // true if selected
		std::string tag; // component name
	};

	void ComponentsPanel::show()
	{
		ImGui::Begin(m_name.c_str(), nullptr,
			ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse);

		//define bounds of the window
		static bool isPositionSet = false;

		if (!isPositionSet)
		{
			ImGui::SetWindowPos(m_name.c_str(), ImVec2(m_position.x, m_position.y));
			isPositionSet = true;
		}

		ImGui::SetWindowSize(m_name.c_str(), ImVec2(m_dimension.x, m_dimension.y));

		//defines the title section above the UI element
		partition("MyriadPro_bold_18", m_name, DARK_CYAN);

		m_isAddButtonClicked = false;

		ImGui::PushFont(s_fonts["MyriadPro_14"]);
		s_style->Colors[ImGuiCol_Text] = OFF_WHITE;

		for (const auto& component : m_components)
		{
			// Initializes ComponentButton struct for each component
			ComponentsButton Component;
			Component.tag = component;
			Component.state = false;

			// if this component is selected, fill in the radio button
			if (Component.tag == m_selectedComponent)
			{
				Component.state = true;
			}

			if (ImGui::RadioButton(Component.tag.c_str(), Component.state))
			{
				m_selectedComponent = Component.tag;
			}
		}

		ImGui::PopFont();

		setSpacing(3);

		ImGui::PushFont(s_fonts["MyriadPro_bold_14"]);
		s_style->Colors[ImGuiCol_Text] = WHITE;

		if (ImGui::Button("Add component to selected entity", ImVec2(250, 25)))
		{
			m_isAddButtonClicked = true;
		}

		ImGui::PopFont();
		ImGui::End();
	}
}