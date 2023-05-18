#include "CinnamonEditor/include/Popups/ModalPopup.hpp"
#include "ThirdParty/imgui/imgui.h"

ModalPopup::ModalPopup(
	const char* title,
	const float width,
	const float height,
	const EModalPopupFlags flags) noexcept
	:
	m_Title(title),
	m_Width(width),
	m_Height(height),
	m_Flags(flags),
	m_IsActive(true)
{}

ModalPopup::~ModalPopup() noexcept
{}

void ModalPopup::OnGUIRender()
{
	if (m_IsActive)
	{
		ImGui::OpenPopup(m_Title);
		
		ImGui::SetNextWindowSizeConstraints(
			ImVec2{ m_Width, m_Height },
			ImVec2{ -1.0f, -1.0f });

		if (m_Flags & EModalPopupFlags::Center)
		{
			ImGui::SetNextWindowPos(
				ImGui::GetMainViewport()->GetCenter(),
				ImGuiCond_Always,
				ImVec2{ 0.5f, 0.5f });
		}

		constexpr ImGuiWindowFlags popupFlags
		{ 
			ImGuiWindowFlags_AlwaysAutoResize	| 
			ImGuiWindowFlags_NoNavInputs		
		};

		if (ImGui::BeginPopupModal(m_Title, nullptr, popupFlags))
		{
			OnGUIRenderInternal();
			ImGui::EndPopup();
		}
	}
}

bool ModalPopup::IsActive()
{
	return m_IsActive;
}

void ModalPopup::Close()
{
	ImGui::CloseCurrentPopup();
	m_IsActive = false;
}
