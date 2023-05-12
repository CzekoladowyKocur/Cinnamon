#pragma once
#include "Cinnamon/include/Renderer/VertexBuffer.hpp"
#include "CinMath/CinMath.h"

namespace Cinnamon {
	namespace Geometry {
		struct QuadVertex final
		{
			CinMath::Vector3 Position;
			CinMath::Vector2 TextureCoordinates;
			
			static constexpr VertexBufferLayout GetLayout() noexcept
			{
				return VertexBufferLayout
				{
					STL::InitializerList<VertexBufferElement>
					{
						VertexBufferElement{ EShaderDataType::Float3 },
						VertexBufferElement{ EShaderDataType::Float2 }
					}
				};
			}
		};
	}
}