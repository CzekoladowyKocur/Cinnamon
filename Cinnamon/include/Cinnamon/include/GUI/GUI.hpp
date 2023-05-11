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
	}
}
