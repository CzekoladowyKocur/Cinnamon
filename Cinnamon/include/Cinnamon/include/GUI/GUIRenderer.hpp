#pragma once
#include "Cinnamon/include/Core/Core.hpp"

namespace Cinnamon {	
	class Renderer;
	class Window;
	class Event;
	enum class EUITheme;

	class GUIRenderer final
	{
	private:
		NON_COPYABLE(GUIRenderer)
	public:
		explicit GUIRenderer(
			const STL::Unique<Window>& window,
			const STL::Unique<Renderer>& renderer) noexcept;
		
		~GUIRenderer() noexcept;

		void BeginFrame();
		void EndFrame();
		void SetTheme(const EUITheme theme);
		void OnEvent(const Event& event);

		[[nodiscard]] const STL::Unique<Renderer>&
			GetRenderer() const;
	private:
		void UploadFontAtlas();
		void UploadIconFontAtlas();
	private:
		const STL::Unique<Renderer>& m_Renderer;
		struct InternalGUIRendererState* m_InternalState;
	};
}