#include "CinnamonEditor/include/Panels/EditorViewportPanel.hpp"
#include "Cinnamon/include/Renderer/Renderer.hpp"
#include "Cinnamon/include/Renderer/Framebuffer.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"
#include "Cinnamon/include/Scene/SceneRenderer.hpp"
#include "Cinnamon/include/Scene/Entity.hpp"
#include "Cinnamon/include/Scene/Components.hpp"
#include "Cinnamon/include/Scene/SceneSerializer.hpp"
#include "Cinnamon/include/Asset/AssetManager.hpp"

#include "Cinnamon/include/GUI/GUI.hpp"
#include "Cinnamon/include/GUI/Icons.hpp"
#include "Cinnamon/include/Event/WindowEvent.hpp"
#include "Cinnamon/include/Event/KeyEvent.hpp"
#include "Cinnamon/include/Event/MouseEvent.hpp"
#include "Cinnamon/include/Core/Input.hpp"
#include "CinMath/CinMath.h"

#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_internal.h"
#include "ThirdParty/ImGuizmo/ImGuizmo.h"

using namespace Cinnamon;
EditorViewportPanel::EditorViewportPanel(
	ProjectContext projectContext,
	SceneContext sceneContext,
	SelectionContext selectionContext,
	const STL::Unique<Renderer>& renderer, 
	const STL::Unique<AssetManager>& assetManager,
	const uint32_t viewportWidth,
	const uint32_t viewportHeight) noexcept
	:
	EditorPanelBase(projectContext, sceneContext, selectionContext),
	m_EditorCamera(static_cast<float>(viewportWidth) / viewportHeight),
	m_GizmoOperation(EGizmoOperation::None),
	m_Renderer(renderer),
	m_AssetManager(assetManager),
	m_SceneRenderer(STL::MakeUnique<SceneRenderer>(renderer, false, viewportWidth, viewportHeight)),
	m_Viewport
	{
		.AspectRatio{ static_cast<float>(viewportWidth) / viewportHeight },
		.Width{ static_cast<float>(viewportWidth) },
		.Height{ static_cast<float>(viewportHeight) },
		.Focused{ false },
		.Hovered{ false },
		.BoundsX{ CinMath::Vector2{ 0.0f, 0.0f } },
		.BoundsY{ CinMath::Vector2{ 0.0f, 0.0f } },
	}
{
	CIN_TRACE("Constructed editor viewport panel");
}

EditorViewportPanel::~EditorViewportPanel() noexcept
{}

void EditorViewportPanel::OnUpdate(const Timestep timestep)
{
	m_SceneRenderer->SetRenderedScene(m_SimulatedScene ? m_SimulatedScene.get() : m_SceneContext);
	m_SceneRenderer->OnUpdate(timestep);
	
	if (m_Renderer)
	{
		m_EditorCamera.SetAspectRatio(m_Viewport.AspectRatio);
		m_SceneRenderer->SetAspectRatio(m_Viewport.AspectRatio);
		m_SceneRenderer->RenderScene(m_EditorCamera.GetViewProjectionMatrix(), m_EditorCamera.GetPosition());
	}

	CIN_UNUSED(timestep);
}

void EditorViewportPanel::OnGUIRender()
{
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ ImGui::GetStyle().Colors[ImGuiCol_FrameBg] });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 4.0f, 0.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, { 0.0f });
	ImGui::Begin(GetPanelName(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	RenderToolbar();
	const ImVec2 viewportPanelSize{ ImGui::GetContentRegionAvail() };
	const ImVec2 viewportMinRegion{ ImGui::GetCursorPos() };
	const ImVec2 viewportMaxRegion{ ImGui::GetContentRegionMax() };
	const ImVec2 viewportOffset{ ImGui::GetWindowPos() };

	m_Viewport.Width	= viewportPanelSize.x;
	m_Viewport.Height	= viewportPanelSize.y;
	
	if(viewportPanelSize.x > 0.0f && viewportPanelSize.y > 0.0f)
		m_Viewport.AspectRatio = viewportPanelSize.x / viewportPanelSize.y;

	m_Viewport.Focused = ImGui::IsWindowFocused();
	m_Viewport.Hovered = ImGui::IsWindowHovered();
	m_Viewport.BoundsX = CinMath::Vector2{ viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
	m_Viewport.BoundsY = CinMath::Vector2{ viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

	RenderViewport();

	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor();
	ImGui::End();
}

void EditorViewportPanel::OnEvent(const Event& event)
{
	const EventDispatcher dispatcher(event);
	dispatcher.Dispatch<KeyPressedEvent>(std::bind(&EditorViewportPanel::OnKeyPressed, this, std::placeholders::_1));
	dispatcher.Dispatch<MousePressedEvent>(std::bind(&EditorViewportPanel::OnMousePressed, this, std::placeholders::_1));
	dispatcher.Dispatch<WindowResizedEvent>(std::bind(&EditorViewportPanel::OnWindowResized, this, std::placeholders::_1));
	
	m_EditorCamera.OnEvent(event, m_Viewport.Hovered);
}

constexpr const char* EditorViewportPanel::GetPanelName() const
{
	return "Editor Viewport";
}

bool EditorViewportPanel::OnKeyPressed(const KeyPressedEvent& event)
{
	/* Gizmos */
	if (m_SelectionContext)
	{
		switch (event.GetKey())
		{
			/* None */
			case Key::Escape:
			case Key::Q:
			{
				m_GizmoOperation = EGizmoOperation::None;
			} break;

			/* Translate */
			case Key::W:
			{
				m_GizmoOperation = EGizmoOperation::Translate;
			} break;

			/* Scale */
			case Key::E:
			{
				m_GizmoOperation = EGizmoOperation::Rotate;
			} break;

			/* Rotate */
			case Key::R:
			{
				m_GizmoOperation = EGizmoOperation::Scale;
			} break;

			default:
				break;
		}
	}

	return false;
}

bool EditorViewportPanel::OnMousePressed(const Cinnamon::MousePressedEvent& event)
{
	CIN_UNUSED(event);
	return false;
}

bool EditorViewportPanel::OnWindowResized(const Cinnamon::WindowResizedEvent& event)
{
	const auto [windowWidth, windowHeight] { event.GetResize()};
	m_SceneRenderer->SetViewportSize(windowWidth, windowHeight);

	return false;
}

void EditorViewportPanel::RenderToolbar()
{
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f });

	if (ImGui::Button(ICON_FA_MOUSE_POINTER) and m_SelectionContext)
		m_GizmoOperation = EGizmoOperation::None;
	
	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_ARROWS_ALT) and m_SelectionContext)
		m_GizmoOperation = EGizmoOperation::Translate;

	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_UNDO) and m_SelectionContext)
		m_GizmoOperation = EGizmoOperation::Rotate;

	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_EXPAND) and m_SelectionContext)
		m_GizmoOperation = EGizmoOperation::Scale;
	
	ImGui::PopStyleColor();
	/* Show FPS. */
	ImGui::SameLine();
	ImGui::Text("FPS: %f\n", ImGui::GetIO().Framerate);
	/* Render Scene Play/Pause/Stop buttons. */
	constexpr ImVec2 buttonSize{ 21.0f, 24.0f };
	const float spacing{ (ImGui::GetWindowWidth() - buttonSize.x * 3.0f) * 0.5f };

	ImGui::BeginDisabled(not m_SceneContext);
	{
		const bool isSceneInPlay{ m_SimulatedScene ? m_SimulatedScene->GetSceneState() == ESceneState::Playing : false };
		const bool isScenePaused{ m_SimulatedScene ? m_SimulatedScene->GetSceneState() == ESceneState::Paused : false};
		const bool isSceneEdited{ m_SimulatedScene ? m_SimulatedScene->GetSceneState() == ESceneState::Edited : false };
		
		ImGui::SameLine();
		ImGui::SetCursorPosX(spacing);
		ImGui::BeginDisabled(isSceneInPlay);
		if (ImGui::Button(ICON_FA_PLAY, buttonSize))
			SetSimulatedSceneState(ESceneState::Playing);
			
		ImGui::EndDisabled();

		ImGui::SameLine();
		ImGui::BeginDisabled(not isSceneInPlay);
		if (ImGui::Button(ICON_FA_PAUSE, buttonSize))
			SetSimulatedSceneState(ESceneState::Paused);

		ImGui::EndDisabled();

		ImGui::BeginDisabled(not isSceneInPlay and not isScenePaused or isSceneEdited);
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_STOP, buttonSize))
			SetSimulatedSceneState(ESceneState::Edited);

		ImGui::EndDisabled();
	}
	ImGui::EndDisabled();
}

void EditorViewportPanel::RenderViewport()
{
	/* Displaying the image will move the cursor so we retrieve it early */
	const ImVec2 screenCursorPosition = ImGui::GetCursorScreenPos();
	/* Viewport is renderer here */
	GUI::Image(
		reinterpret_cast<ImageViewID>(m_SceneRenderer->GetFramebuffer()->GetColorAttachmentView(0U)),
		m_Viewport.Width,
		m_Viewport.Height,
		reinterpret_cast<ImageSamplerID>(m_SceneRenderer->GetFramebuffer()->GetSampler()), true);

	/* Accept scene payload */
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* const payload{ ImGui::AcceptDragDropPayload("ScenePayload") })
		{
			const STL::Filepath scenePath(reinterpret_cast<const char*>(payload->Data));
			if (m_SceneContext)
				cindel m_SceneContext;

			m_SelectionContext = Entity();
			m_SceneContext = cinew Scene(ESceneState::Edited);
			if (not (SceneSerializer(m_SceneContext, m_AssetManager) << scenePath))
				CIN_ERROR("Failed loading a dragged scene with path {}", scenePath.string());
		}

		ImGui::EndDragDropTarget();
	}
	
	if (m_SelectionContext and m_Viewport.Hovered and m_GizmoOperation != EGizmoOperation::None)
	{
		/* Render gizmos if used */
		ImGuizmo::SetOrthographic(true);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(
			screenCursorPosition.x,
			screenCursorPosition.y,
			m_Viewport.Width,
			m_Viewport.Height);

		TransformComponent& transformComponent{ m_SelectionContext.GetComponent<TransformComponent>() };
		CinMath::Matrix4 transform(transformComponent.Calculate());

		ImGuizmo::Manipulate(
			m_EditorCamera.GetViewMatrix(),
			m_EditorCamera.GetProjectionMatrix(),
			static_cast<ImGuizmo::OPERATION>(m_GizmoOperation),
			ImGuizmo::MODE::LOCAL,
			transform);

		if (ImGuizmo::IsUsing())
		{
			CinMath::Vector3 translation{ 0.0f }, rotation{ 0.0f }, scale{ 0.0f };
			ImGuizmo::DecomposeMatrixToComponents(transform, translation, rotation, scale);

			transformComponent.Translation = translation;
			transformComponent.Rotation = -rotation;
			transformComponent.Scale = scale;
		}
	}
}

void EditorViewportPanel::SetSimulatedSceneState(const ESceneState sceneState)
{
	CIN_ASSERT(m_SceneContext);
	switch (sceneState)
	{
		case ESceneState::Paused:
		{
			CIN_ASSERT(m_SimulatedScene);
			m_SimulatedScene->SetSceneState(ESceneState::Paused);
		} break;

		case ESceneState::Playing:
		{
			if (m_SimulatedScene and m_SimulatedScene->GetSceneState() == ESceneState::Paused)
			{
				m_SimulatedScene->SetSceneState(ESceneState::Playing);
			}
			else
			{
				FunctionVariable constexpr const char* simulatedSceneFile{ "SimulatedScene.cinscene" };
				if (not (SceneSerializer(m_SceneContext, m_AssetManager) >> simulatedSceneFile))
				{
					CIN_ERROR("Failed to serialize scene for runtime");
				}
				else
				{
					m_SimulatedScene = STL::MakeUnique<Scene>(ESceneState::Playing);
					if (not (SceneSerializer(m_SimulatedScene.get(), m_AssetManager) << simulatedSceneFile))
					{
						CIN_ERROR("Failed to deserialize scene for runtime");
						m_SimulatedScene.reset();
					}

					m_SimulatedScene->SetSceneState(ESceneState::Playing);
				}
			}
			
			m_SceneRenderer->SetRenderedScene(m_SimulatedScene.get());
		} break;

		case ESceneState::Edited:
		{
			CIN_ASSERT(m_SimulatedScene);
			m_SimulatedScene.reset();
			m_SceneRenderer->SetRenderedScene(m_SceneContext);
		} break;
	}
}