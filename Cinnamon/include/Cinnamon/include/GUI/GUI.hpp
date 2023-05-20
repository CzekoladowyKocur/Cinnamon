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
			const float min,
			const float max,
			const float width);

		void Vec1Slider(
			const STL::StringView label,
			float values[1U],
			const float resetValue,
			const float min,
			const float max,
			const float width);

		void Vec2Slider(
			const STL::StringView label,
			float values[2U],
			const float min,
			const float max,
			const float width);

		void Vec2Slider(
			const STL::StringView label,
			float values[2U],
			const float resetValue,
			const float min,
			const float max,
			const float width);

		void Vec3Slider(
			const STL::StringView label,
			float values[3U],
			const float resetValue,
			const float min,
			const float max,
			const float width);

		void ColorPicker4(
			const STL::StringView label,
			float values[4U],
			const float width);
	}
}
