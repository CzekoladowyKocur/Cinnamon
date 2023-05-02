#include "Cinnamon/include/Core/LayerStack.hpp"

namespace Cinnamon {
	LayerStack::LayerStack() noexcept
		:
		m_LayerStack(),
		m_LayerInsertionIndex(0U)
	{}
	
	LayerStack::~LayerStack() noexcept
	{
		m_LayerStack.clear();
	}

	void LayerStack::PushLayer(Layer* const layer)
	{
		m_LayerStack.emplace(m_LayerStack.cbegin() + m_LayerInsertionIndex, layer);
		++m_LayerInsertionIndex;
	}

	void LayerStack::PopLayer(Layer* const layer)
	{
		const auto iterator{ std::find_if(m_LayerStack.cbegin(), m_LayerStack.cend(), [=](Layer* const layerIterator) {
			return layerIterator == layer;
			}) };

		CIN_ASSERT(iterator != m_LayerStack.end());
		m_LayerStack.erase(iterator);
		--m_LayerInsertionIndex;
	}

	void LayerStack::PushOverlay(Layer* const layer)
	{
		m_LayerStack.push_back(layer);
	}

	void LayerStack::PopOverlay(Layer* const layer)
	{
		const auto iterator{ std::find_if(m_LayerStack.cbegin(), m_LayerStack.cend(), [=](Layer* const layerIterator) {
			return layerIterator == layer;
			}) };

		CIN_ASSERT(iterator != m_LayerStack.end());
		m_LayerStack.erase(iterator);
	}

	std::vector<Layer*>::iterator LayerStack::begin()
	{
		return m_LayerStack.begin();
	}

	std::vector<Layer*>::iterator LayerStack::end()
	{
		return m_LayerStack.end();
	}

	std::vector<Layer*>::const_iterator LayerStack::cbegin()
	{
		return m_LayerStack.cbegin();
	}

	std::vector<Layer*>::const_iterator LayerStack::cend()
	{
		return m_LayerStack.cend();
	}
}
