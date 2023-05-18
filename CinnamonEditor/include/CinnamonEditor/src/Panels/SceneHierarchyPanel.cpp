#include "CinnamonEditor/include/Panels/SceneHierarchyPanel.hpp"
#include "Cinnamon/include/GUI/GUI.hpp"
#include "Cinnamon/include/GUI/Icons.hpp"
#include "Cinnamon/include/Scene/Scene.hpp"
#include "Cinnamon/include/Scene/Components.hpp"
#include "Cinnamon/include/ECS/Registry.hpp"

#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_internal.h"

using namespace Cinnamon;
SceneHierarchyPanel::SceneHierarchyPanel(
	Project*& projectContext,
	Scene*& sceneContext, 
	Entity& selectionContext) noexcept
	:
	EditorPanelBase(projectContext, sceneContext, selectionContext),
	m_HierarchyTableRowColors({})
{
	const ImVec4 firstColor{ ImGui::GetStyle().Colors[ImGuiCol_FrameBg] };
	const ImVec4 secondColor{ firstColor.x * 1.4f, firstColor.y * 1.4f, firstColor.z * 1.4f, 1.0f };

	m_HierarchyTableRowColors.First = ImGui::GetColorU32(firstColor);
	m_HierarchyTableRowColors.Second = ImGui::GetColorU32(secondColor);
}

void SceneHierarchyPanel::OnUpdate(const Timestep timestep)
{
	CIN_UNUSED(timestep);
}

void SceneHierarchyPanel::OnGUIRender()
{
	ImGui::PushStyleColor(ImGuiCol_WindowBg, m_HierarchyTableRowColors.First);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });

	ImGui::Begin(GetPanelName());

	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	
	/* Search bar, used for filtering entities with matching names. */
	FunctionVariable STL::String outSearch;
	/* Update discarded entities if search phrase was updated*/
	if (GUI::SearchBar(outSearch, true))
	{
		m_SearchDiscaredEntities.clear();
		/* Update the list if a non-empty search phrase is active */
		if (not outSearch.empty() and m_SceneContext)
		{
			for (const ECS::EntityID entityID : ECS::View(m_SceneContext->GetRegistry()))
			{
				Entity entity{ entityID, m_SceneContext };
				const auto& tag{ entity.GetComponent<TagComponent>().Tag };

				if (tag.find(outSearch) == STL::String::npos)
					m_SearchDiscaredEntities.emplace(entityID);
			}
		}
	}
	else if (outSearch.empty())
		m_SearchDiscaredEntities.clear();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 2.0f, 2.0f });
	/* Draw entity hierarchy. */
	if (ImGui::BeginTable("##hierarchy_table", 2))
	{
		ImGui::TableSetupColumn(" Label");
		ImGui::TableSetupColumn(" Type");
		ImGui::TableHeadersRow();

		if (m_SceneContext)
		{
			ImGui::Unindent(ImGui::GetStyle().IndentSpacing * 2.0f);
			
			bool flip{ false };
			/* Begin a filtered search */
			if (not outSearch.empty())
			{
				for (const ECS::EntityID entityID : ECS::View(m_SceneContext->GetRegistry()))
				{
					if(m_SearchDiscaredEntities.contains(entityID))
						continue;

					const uint32_t rowColor{ flip ? m_HierarchyTableRowColors.First : m_HierarchyTableRowColors.Second };
					flip = !flip;

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, rowColor);
					DrawEntityNode(Entity{ entityID, m_SceneContext });

					ImGui::TableSetColumnIndex(1);
					ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, rowColor);
					ImGui::TextUnformatted(ICON_FA_CUBE);
				}
			}
			else
			{
				for (const ECS::EntityID entityID : ECS::View(m_SceneContext->GetRegistry()))
				{
					const uint32_t rowColor{ flip ? m_HierarchyTableRowColors.First : m_HierarchyTableRowColors.Second };
					flip = !flip;

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, rowColor);
					DrawEntityNode(Entity{ entityID, m_SceneContext });
					
					ImGui::TableSetColumnIndex(1);
					ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, rowColor);
					ImGui::TextUnformatted(ICON_FA_CUBE);
				}
			}
		}
		
		ImGui::EndTable();
	}

	/* Reset selection. */
	if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
		m_SelectionContext = Entity();

	/* Create entity popup. */
	if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
	{
		if (ImGui::Button("Create Empty Entity"))
		{
			m_SelectionContext = m_SceneContext->CreateEntity("Unnamed Entity");
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::PopStyleVar();
	ImGui::End();
}

void SceneHierarchyPanel::OnEvent(const Event& event)
{
	CIN_UNUSED(event);
}

constexpr const char* SceneHierarchyPanel::GetPanelName() const
{
	return "Scene Hierarchy Panel";
}

void SceneHierarchyPanel::DrawEntityNode(Entity entity)
{
	ImGuiTreeNodeFlags conditionalFlags{ ImGuiTreeNodeFlags_None };
	if (m_SelectionContext == entity)
	{
		ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]));
		conditionalFlags |= ImGuiTreeNodeFlags_Selected;
	}

	const ImGuiTreeNodeFlags treeNodeFlags
	{ 
		ImGuiTreeNodeFlags_SpanFullWidth	|	 
		ImGuiTreeNodeFlags_Leaf				| 
		ImGuiTreeNodeFlags_DefaultOpen		|	 
		conditionalFlags 
	};

	CIN_ASSERT(entity.HasComponent<TagComponent>());
	const auto& tag{ entity.GetComponent<TagComponent>().Tag };
	const bool entityOpened{ ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<uint64_t>(entity)), treeNodeFlags, tag.data()) };

	if (ImGui::IsItemClicked())
		m_SelectionContext = entity;

	bool wasEntityDeleted{ false };
	if (ImGui::BeginPopupContextItem())
	{
		wasEntityDeleted = ImGui::Button("Delete Entity");

		ImGui::EndPopup();
	}

	if (entityOpened)
	{
		ImGui::TreePop();
	}

	if (wasEntityDeleted)
	{
		m_SceneContext->DestroyEntity(entity);
		m_SelectionContext = Entity();
	}
}