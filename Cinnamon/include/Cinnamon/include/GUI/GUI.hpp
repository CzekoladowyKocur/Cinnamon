#pragma once
#include "Cinnamon/include/Core/Core.hpp"

namespace Cinnamon {
	class Renderer;
	using ImageViewID = uint64_t;

	namespace GUI {
		void Image(
			const STL::Unique<Renderer>& renderer,
			const ImageViewID imageViewID,
			const float width,
			const float height);

		bool SearchBar(
			STL::String& outSearch, 
			const bool spanAvailableWidth);

		void Vec3Slider(
			const STL::StringView label,
			float values[3U],
			float resetValue,
			float width);
	}
}
