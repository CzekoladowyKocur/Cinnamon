#include "CinnamonEditor/include/Panels/EntityPropertiesPanel.hpp"
#include "CinnamonEditor/include/Popups/Texture2DImportPopup.hpp"
#include "Cinnamon/include/Scene/Components.hpp"
#include "Cinnamon/include/Renderer/Texture2D.hpp"
#include "Cinnamon/include/Asset/AssetManager.hpp"

#include "Cinnamon/include/GUI/GUI.hpp"
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_internal.h"

using namespace Cinnamon;
template<typename Component>
void DrawAddComponent(Entity entity, const STL::StringView componentName) noexcept;

template<typename Component, typename Function>
void DrawComponent(Entity entity, const STL::StringView componentName, const bool removable, Function&& function) noexcept;

template<typename Function>
void DrawComponentRow(const char* label, Function&& function) noexcept;

InternalScope constexpr const char* TextureFilterModeToString(const ETextureSamplerFilterMode filterMode);
InternalScope constexpr const char* TextureWrapModeToString(const ETextureSamplerWrapMode wrapMode);

EntityPropertiesPanel::EntityPropertiesPanel(
	Project*& projectContext,
	Scene*& sceneContext, 
	Entity& selectionContext,
	const Cinnamon::STL::Unique<Cinnamon::AssetManager>& assetManager) noexcept
	:
	EditorPanelBase(projectContext, sceneContext, selectionContext),
	m_AssetManager(assetManager),
	m_ModalPopup(nullptr),
	m_EmptyTexture(m_AssetManager->LoadTexture("Resources/textures/paper.png", TextureSpecification{}))
{}

EntityPropertiesPanel::~EntityPropertiesPanel() noexcept
{}

void EntityPropertiesPanel::OnUpdate(const Timestep timestep)
{
	CIN_UNUSED(timestep);
}

void EntityPropertiesPanel::OnGUIRender()
{
	/* Disable any padding to make the collapsing headers fill all the space. */
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
	ImGui::Begin(GetPanelName());

	if (m_SelectionContext)
	{
		DrawEntityComponents(m_SelectionContext);
	}

	ImGui::End();
	ImGui::PopStyleVar();

	if (m_ModalPopup)
	{
		if (m_ModalPopup->IsActive())
			m_ModalPopup->OnGUIRender();
		else
			m_ModalPopup.reset();
	}
}

void EntityPropertiesPanel::OnEvent(const Event& event)
{
	CIN_UNUSED(event);
}

constexpr const char* EntityPropertiesPanel::GetPanelName() const
{
	return "Entity Properties";
}

void EntityPropertiesPanel::DrawEntityComponents(Cinnamon::Entity entity)
{
	CIN_ASSERT(entity.HasComponent<TagComponent>());
	STL::String& tag{ entity.GetComponent<TagComponent>().Tag };
	
	const ImVec2 contentRegionAvailable{ ImGui::GetContentRegionAvail() };
	const float simulatedWindowPaddingWidth{ 4.0f };

	ImGui::PushItemWidth(contentRegionAvailable.x * 0.55f);
	ImGui::SetCursorPos({ ImGui::GetCursorPosX() + simulatedWindowPaddingWidth, ImGui::GetCursorPosY() });
	char buffer[256U]{ '\0' };
	memcpy(buffer, tag.data(), tag.size());
	if (ImGui::InputText("##EntityTag", buffer, 256U))
		tag = STL::String(buffer);

	const ImVec2 textSize{ ImGui::CalcTextSize("Add Component") };
	ImGui::SameLine(contentRegionAvailable.x - (textSize.x + GImGui->Style.FramePadding.y));

	ImGui::SetCursorPos({ ImGui::GetCursorPosX() - 2.0f * simulatedWindowPaddingWidth, ImGui::GetCursorPosY() });
	if (ImGui::Button("Add Component"))
		ImGui::OpenPopup("##AddComponentPopup");

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 4.0f, 4.0f });
	if (ImGui::BeginPopup("##AddComponentPopup"))
	{
		ImGui::Text("Available components");
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);

		DrawAddComponent<SpriteRendererComponent>(entity, "Sprite renderer");
		DrawAddComponent<PointLightComponent>(entity, "Point light");
		DrawAddComponent<RigidBody2DComponent>(entity, "Rigid body 2D");
		DrawAddComponent<Box2DColliderComponent>(entity, "Box collider 2D");

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();

	ImGui::PopItemWidth();

	/* Disable any spacing to make the collapsing headers fill all the space. */
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 1.0f });	
	/* Set the size of the collapsing header. */
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 1.0f, 2.0f });
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ 0.09f, 0.09f, 0.09f, 1.0f });
	/* Set the color of the table borders. */
	ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TableBorderLight, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

	DrawComponent<TransformComponent>(entity, "Transform", false, [](Entity /*entity*/, TransformComponent& component)
	{
		CinMath::Vector3& translation{ component.Translation };
		DrawComponentRow("Translation", [&translation]()
		{
			GUI::Vec3Slider("Translation", translation, 0.0f, -1e9f, 1e9f, ImGui::GetContentRegionAvail().x);
		});

		CinMath::Vector3& scale{ component.Scale };
		DrawComponentRow("Scale", [&scale]()
		{
			GUI::Vec3Slider("Scale", scale, 1.0f, 0.0f, 1e9f, ImGui::GetContentRegionAvail().x);
		});

		CinMath::Vector3& rotation{ component.Rotation };
		DrawComponentRow("Rotation", [&rotation]()
		{
			GUI::Vec3Slider("Rotation", rotation, 0.0f, -1e9f, 1e9f, ImGui::GetContentRegionAvail().x);
		});
	});

	DrawComponent<SpriteRendererComponent>(entity, "Sprite", true, [this](Entity /*entity*/, SpriteRendererComponent& component)
	{
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
		const Texture2D* const texture{ component.Texture ? component.Texture : m_EmptyTexture };
		DrawComponentRow("Texture", [&, this]()
		{
			const auto [textureWidth, textureHeight]{ texture->GetSize() };
			const float textureAspectRatio{ static_cast<float>(textureWidth) / textureHeight };
			const float availableWidth{ ImGui::GetContentRegionAvail().x };

			GUI::Image(
				reinterpret_cast<ImageViewID>(texture->GetImageView()), 
				availableWidth, 
				availableWidth / textureAspectRatio,
				reinterpret_cast<ImageSamplerID>(texture->GetSampler()));
			
			if (not component.Texture)
			{
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* const payload{ ImGui::AcceptDragDropPayload("PNGImagePayload") })
					{
						const char* path{ reinterpret_cast<const char*>(payload->Data) };

						STL::Unique<Texture2DImportPopup> importPopup{ STL::MakeUnique<Texture2DImportPopup>(path) };
						importPopup->SetImportCallback([&](const STL::Filepath& filepath, const TextureSpecification& textureSpeficitation)
						{
							if (!(component.Texture = m_AssetManager->LoadTexture(filepath, textureSpeficitation)))
								CIN_WARN("Failed loading texture with path {}", path);
						});

						m_ModalPopup = std::move(importPopup);
					}

					ImGui::EndDragDropTarget();
				}
			}
		});

		const TextureSpecification& textureSpecification{ texture->GetSpecification() };
		DrawComponentRow("Wrap mode", [&textureSpecification]()
		{
			int currentItem{ 0 };
			ImGui::BeginDisabled();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::Combo("##FilterMode", &currentItem, TextureWrapModeToString(textureSpecification.SamplerWrapMode));
			ImGui::EndDisabled();
		});

		DrawComponentRow("Filter mode", [&textureSpecification]()
		{
			int currentItem{ 0 };
			ImGui::BeginDisabled();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::Combo("##FilterMode", &currentItem, TextureFilterModeToString(textureSpecification.SamplerFilterMode));
			ImGui::EndDisabled();
		});

		DrawComponentRow("Color", [&]()
		{
			GUI::ColorPicker4("##SpriteColor", component.Color, ImGui::GetContentRegionAvail().x);
		});

		DrawComponentRow("Tiling Factor", [&]()
		{
			GUI::Vec1Slider("Tiling factor", &component.TilingFactor, 1e-9f, 1e9f, ImGui::GetContentRegionAvail().x);
		});
	});

	DrawComponent<PointLightComponent>(entity, "Point light", true, [](Entity /*entity*/, PointLightComponent& component)
	{
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
		DrawComponentRow("Color", [&]()
		{
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::ColorEdit4("##PointLightColor", component.Color, ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_PickerHueWheel);
			ImGui::PopItemWidth();
		});

		DrawComponentRow("Intensity", [&]() 
		{
			GUI::Vec1Slider("Intensity", &component.Intensity, 1.0f, 0.0f, 1e9f, ImGui::GetContentRegionAvail().x);
		});
	});

	DrawComponent<RigidBody2DComponent>(entity, "Rigid Body 2D", true, [](Entity /*entity*/, RigidBody2DComponent& component)
	{
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
		DrawComponentRow("Body Type", [&]()
		{
			FunctionVariable constexpr const char* f_BodyTypes[]{ "Static", "Kinematic", "Dynamic" };
			static_assert(static_cast<uint32_t>(EBodyType::Static)		== 0U);
			static_assert(static_cast<uint32_t>(EBodyType::Kinematic)	== 1U);
			static_assert(static_cast<uint32_t>(EBodyType::Dynamic)		== 2U);

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			int current = { static_cast<int>(component.BodyType) };
			if (ImGui::Combo("##RigidBody2DType", &current, f_BodyTypes, CIN_CARRAY_SIZE(f_BodyTypes)))
				component.BodyType = static_cast<EBodyType>(current);
		});

		DrawComponentRow("Offset", [&]() 
		{
			GUI::Vec2Slider("##RigidBody2DOffset", component.Offset, -1e9, 1e9, ImGui::GetContentRegionAvail().x);
		});

		DrawComponentRow("Angle", [&]()
		{
			GUI::Vec1Slider("##RigidBody2DAngle", &component.Angle, -1e9, 1e9, ImGui::GetContentRegionAvail().x);
		});
	});

	DrawComponent<Box2DColliderComponent>(entity, "Box collider 2D", true, [](Entity /*entity*/, Box2DColliderComponent& component)
	{
		DrawComponentRow("Size", [&]()
		{
			GUI::Vec2Slider("##BoxCollider2DSize", component.Size, 0.0f, 1e9, ImGui::GetContentRegionAvail().x);
		});
	});
	
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(3);
}

template<typename Component, typename Function>
void DrawComponent(Entity entity, const STL::StringView componentName, const bool removable, Function&& function) noexcept
{
	if (entity.HasComponent<Component>())
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
		const bool opened{ ImGui::CollapsingHeader(componentName.data(), ImGuiTreeNodeFlags_SpanFullWidth) };
		ImGui::PopStyleVar();

		if (removable)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 2.0f, 2.0f });
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 4.0f, 10.0f });

			if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
			{
				if (ImGui::Button("Remove component"))
				{
					entity.RemoveComponent<Component>();
					ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
					ImGui::PopStyleVar(2);
					return;
				}

				ImGui::EndPopup();
			}

			ImGui::PopStyleVar(2);
		}

		if (opened)
		{
			const STL::String tableID{ "##table_" + STL::String(componentName) };
			if(ImGui::BeginTable(componentName.data(), 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersInnerH))
			{
				ImGui::TableSetupColumn("##LabelColumn", ImGuiTableFlags_None, ImGui::GetWindowSize().x * 0.33f);
				ImGui::TableSetupColumn("##ComponentColumn", ImGuiTableFlags_None, ImGui::GetWindowSize().x * 0.67f);

				function(entity, entity.GetComponent<Component>());
				ImGui::EndTable();
			}
		}
	}
}

template<typename Component>
void DrawAddComponent(Entity entity, const STL::StringView componentName) noexcept
{
	if (not entity.HasComponent<Component>())
	{
		if (ImGui::Button(componentName.data()))
		{
			entity.AddComponent<Component>();
			ImGui::CloseCurrentPopup();
		}
	}
}

template<typename Function>
void DrawComponentRow(const char* label, Function&& function) noexcept
{
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::TextUnformatted(label);
	
	ImGui::TableSetColumnIndex(1);
	function();
}

InternalScope constexpr const char* TextureFilterModeToString(const ETextureSamplerFilterMode filterMode)
{
	switch (filterMode)
	{
		case ETextureSamplerFilterMode::Linear:		return "Linear";
		case ETextureSamplerFilterMode::Nearest:	return "Nearest";

		default:
		{
			CIN_ASSERT(false);
			return "Unknown";
		}
	}
}

InternalScope constexpr const char* TextureWrapModeToString(const ETextureSamplerWrapMode wrapMode)
{
	switch (wrapMode)
	{
		case ETextureSamplerWrapMode::Clamp:	return "Clamp";
		case ETextureSamplerWrapMode::Repeat:	return "Repeat";

		default:
		{
			CIN_ASSERT(false);
			return "Unknown";
		}
	}
}