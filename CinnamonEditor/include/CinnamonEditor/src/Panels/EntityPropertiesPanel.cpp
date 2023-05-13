#include "CinnamonEditor/include/Panels/EntityPropertiesPanel.hpp"
#include "Cinnamon/include/Scene/Components.hpp"

#include "Cinnamon/include/GUI/GUI.hpp"
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_internal.h"

using namespace Cinnamon;
EntityPropertiesPanel::EntityPropertiesPanel(Scene*& sceneContext, Entity& selectionContext) noexcept
	:
	EditorPanelBase(sceneContext, selectionContext)
{}

EntityPropertiesPanel::~EntityPropertiesPanel() noexcept
{}

void EntityPropertiesPanel::OnUpdate(const Timestep timestep)
{
	CIN_UNUSED(timestep);
}

void EntityPropertiesPanel::OnGUIRender()
{
	ImGui::Begin(GetPanelName());
	if (m_SelectionContext)
	{
		DrawEntityComponents(m_SelectionContext);
	}

	ImGui::End();
}

void EntityPropertiesPanel::OnEvent(const Event& event)
{
	CIN_UNUSED(event);
}

constexpr const char* EntityPropertiesPanel::GetPanelName() const
{
	return "Entity Properties Panel";
}

void EntityPropertiesPanel::DrawEntityComponents(Cinnamon::Entity entity)
{
	CIN_ASSERT(entity.HasComponent<TagComponent>());
	STL::String& tag{ entity.GetComponent<TagComponent>().Tag };

	const ImVec2 contentRegionAvailable{ ImGui::GetContentRegionAvail() };

	ImGui::PushItemWidth(contentRegionAvailable.x * 0.55f);
	char buffer[256U]{ '\0' };
	memcpy(buffer, tag.data(), tag.size());
	if(ImGui::InputText("##EntityTag", buffer, 256U))
		tag = STL::String(buffer);

	const ImVec2 textSize{ ImGui::CalcTextSize("Add Component") };
	ImGui::SameLine(contentRegionAvailable.x - (textSize.x + GImGui->Style.FramePadding.y));

	if (ImGui::Button("Add Component"))
	{ 
	}

	ImGui::PopItemWidth();

	/* Set the size of the collapsing header. */
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 1.0f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ 0.09f, 0.09f, 0.09f, 1.0f });

	if (BeginCollapsingHeader("Transform"))
	{
		DrawCollapsingHeaderRow("Translation", [this]()
		{
			TransformComponent& transform{ m_SelectionContext.GetComponent<TransformComponent>() };
			GUI::Vec3Slider("Translation", transform.Position.raw, 0.0f, ImGui::GetContentRegionAvail().x);
		});

		EndCollapsingHeader();
	}

	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
}

bool EntityPropertiesPanel::BeginCollapsingHeader(const Cinnamon::STL::StringView label)
{
	if (ImGui::CollapsingHeader(label.data()))
	{
		const STL::String tableID{ "##table_" + STL::String(label) };
		return ImGui::BeginTable(label.data(), 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV);
	}
	
	return false;
}

void EntityPropertiesPanel::EndCollapsingHeader()
{
	ImGui::EndTable();
}

void EntityPropertiesPanel::DrawCollapsingHeaderRow(const Cinnamon::STL::StringView label, const std::function<void()> functor)
{
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::TextUnformatted(label.data());

	ImGui::TableSetColumnIndex(1);
	functor();
}