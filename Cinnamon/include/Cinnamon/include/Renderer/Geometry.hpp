#pragma once
#include "Cinnamon/include/Renderer/VertexBuffer.hpp"
#include "CinMath/CinMath.h"

namespace Cinnamon {
	namespace Geometry {
		struct QuadVertex final
		{
			CinMath::Vector3 Position;
			CinMath::Vector4 Color;
			CinMath::Vector2 TextureCoordinates;
			float			 TilingFactor;
			
			static VertexBufferLayout GetLayout() noexcept
			{
				return VertexBufferLayout
				{
					STL::InitializerList<VertexBufferElement>
					{
						VertexBufferElement{ EShaderDataType::Float3 },
						VertexBufferElement{ EShaderDataType::Float4 },
						VertexBufferElement{ EShaderDataType::Float2 },
						VertexBufferElement{ EShaderDataType::Float1 }
					}
				};
			}
		};
	}
}