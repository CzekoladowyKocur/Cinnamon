#pragma once
#include "Cinnamon/include/Core/Core.h"

namespace Cinnamon {
	class Window;
}

namespace Cinnamon {
	enum class EUITheme
	{
		Dark,
		Default = Dark,
	};

	class GUIRenderer
	{
	private:
		NON_CONSTRUCTIBLE(GUIRenderer)
		NON_COPYABLE(GUIRenderer)
	public:
		[[nodiscard]] static bool Initialize(const Window* const window);
		[[nodiscard]] static bool Shutdown ();
			
		static void BeginFrame();
		static void EndFrame();
		static void SetTheme(const EUITheme theme);

		[[nodiscard]] static float GetFontSize();
		[[nodiscard]] static float GetIconFontSize();
	private:
		static void UploadFontAtlas();
		static void UploadIconFontAtlas();
	};
}