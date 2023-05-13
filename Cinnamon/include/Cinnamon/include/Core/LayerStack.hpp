#pragma once
#include "Cinnamon/include/Core/Layer.hpp"

namespace Cinnamon {
	class LayerStack final
	{
	private:
		NON_COPYABLE(LayerStack)
	public:
		LayerStack() noexcept;
		~LayerStack() noexcept;

		void PushLayer(Layer* const layer);
		void PopLayer(Layer* const layer);
		void PushOverlay(Layer* const layer);
		void PopOverlay(Layer* const layer);

		[[nodiscard]] std::vector<Layer*>::iterator begin();
		[[nodiscard]] std::vector<Layer*>::iterator end();

		[[nodiscard]] std::vector<Layer*>::const_iterator cbegin();
		[[nodiscard]] std::vector<Layer*>::const_iterator cend();
	private:
		/* Layer stack doesn't own layers */
		STL::Vector<Layer*> m_LayerStack;
		std::size_t m_LayerInsertionIndex;
	};
}