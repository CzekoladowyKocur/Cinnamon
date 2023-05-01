#pragma once
#include "Cinnamon/include/Core/Core.h"

namespace Cinnamon {
	class Renderer;
}

namespace Cinnamon {
	enum class EUITheme
	{
		Dark,
		Default = Dark,
	};

	class GUIRenderer final
	{
	private:
		NON_COPYABLE(GUIRenderer)
	public:
		explicit GUIRenderer(const STL::Unique<Renderer>& renderer);
		~GUIRenderer();

		void BeginFrame();
		void EndFrame();
		void SetTheme(const EUITheme theme);

		[[nodiscard]] static float GetFontSize();
		[[nodiscard]] static float GetIconFontSize();
	private:
		static void UploadFontAtlas();
		static void UploadIconFontAtlas();
	private:
		const STL::Unique<Renderer>& m_Renderer;
		struct InternalGUIRendererState* m_InternalState;
	};
}