#pragma once
#include "Cinnamon/include/Core/Core.h"

namespace Cinnamon {
	class Renderer;
	class Event;
	enum class EUITheme;
}

namespace Cinnamon {	
	class GUIRenderer final
	{
	private:
		NON_COPYABLE(GUIRenderer)
	public:
		explicit GUIRenderer(const STL::Unique<Renderer>& renderer) noexcept;
		~GUIRenderer() noexcept;

		void BeginFrame();
		void EndFrame();
		void SetTheme(const EUITheme theme);
		void OnEvent(const Event& event);
	private:
		void UploadFontAtlas();
		void UploadIconFontAtlas();
	private:
		const STL::Unique<Renderer>& m_Renderer;
		struct InternalGUIRendererState* m_InternalState;
	};
}