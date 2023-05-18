#pragma once
#include "Cinnamon/include/Core/Core.hpp"

namespace Cinnamon {
	class Renderer;
	using ImageViewID = uint64_t;
	using ImageSamplerID = uint64_t;

	namespace GUI {
		void Image(
			const ImageViewID imageViewID,
			const float width,
			const float height,
			const ImageSamplerID sampler,
			const bool flip = false);

		bool SearchBar(
			STL::String& outSearch, 
			const bool spanAvailableWidth);

		void Vec1Slider(
			const STL::StringView label,
			float values[1U],
			float resetValue,
			float width);

		void Vec3Slider(
			const STL::StringView label,
			float values[3U],
			float resetValue,
			float width);
	}
}
