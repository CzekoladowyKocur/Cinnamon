#include "CinnamonEditor/include/Popups/Texture2DImportPopup.hpp"
#include "ThirdParty/imgui/imgui.h"

using namespace Cinnamon;
Texture2DImportPopup::Texture2DImportPopup(const STL::Filepath& importPath) noexcept
	:
	ModalPopup("Import texture", 500.0f, 300.0f, EModalPopupFlags::Center),
	m_ImportPath(importPath),
	m_CancelCallback(nullptr),
	m_ImportCallback(nullptr)
{}

Texture2DImportPopup::~Texture2DImportPopup() noexcept
{}

void Texture2DImportPopup::OnGUIRenderInternal()
{
	/* Texture specification is static to remember last settings */
	FunctionVariable TextureSpecification f_TextureSpecification{ ETextureSamplerWrapMode::Repeat, ETextureSamplerFilterMode::Linear };

	ImGui::TextUnformatted("Texture wrap mode");
	int current{ static_cast<int>(f_TextureSpecification.SamplerWrapMode) };
	if (ImGui::Combo("##WrapModeSelection", &current, s_WrapModes, CIN_CARRAY_SIZE(s_WrapModes)))
		f_TextureSpecification.SamplerWrapMode = static_cast<ETextureSamplerWrapMode>(current);
	
	ImGui::TextUnformatted("Texture filter mode");
	current = { static_cast<int>(f_TextureSpecification.SamplerFilterMode) };
	if(ImGui::Combo("##FilterModeSelection", &current, s_FilterModes, CIN_CARRAY_SIZE(s_FilterModes)))
		f_TextureSpecification.SamplerFilterMode = static_cast<ETextureSamplerFilterMode>(current);

	if (ImGui::Button("Import"))
	{
		if (m_ImportCallback and std::filesystem::exists(m_ImportPath))
			m_ImportCallback(m_ImportPath, f_TextureSpecification);

		Close();
	}

	ImGui::SameLine();
	if (ImGui::Button("Close"))
	{
		if (m_CancelCallback)
			m_CancelCallback();

		Close();
	}
}

void Texture2DImportPopup::SetImportCallback(const Texture2DImportCallback& callback)
{
	m_ImportCallback = callback;
}

void Texture2DImportPopup::SetCancelCallback(const Texture2DCancelCallback& callback)
{
	m_CancelCallback = callback;
}