#include "Cinnamon/include/Core/Layer.hpp"

namespace Cinnamon {
	class Window;
	class Renderer;
	class SceneRenderer;
	class AssetManager;
	class Scene;
	class SceneCamera;
}

class RuntimeLayer final : public Cinnamon::Layer
{
private:
	NON_COPYABLE(RuntimeLayer)
public:
	explicit RuntimeLayer(const Cinnamon::STL::Unique<Cinnamon::Window>& window) noexcept;
	~RuntimeLayer() noexcept;

	virtual void OnAttach() final override;
	virtual void OnUpdate(const Timestep timestep) final override;
	virtual void OnDetach() final override;
	virtual void OnEvent(const Cinnamon::Event& event) final override;
private:
	const Cinnamon::STL::Unique<Cinnamon::Window>& m_Window;
	
	Cinnamon::STL::Unique<Cinnamon::Renderer> m_Renderer;
	Cinnamon::STL::Unique<Cinnamon::SceneRenderer> m_SceneRenderer;
	Cinnamon::STL::Unique<Cinnamon::AssetManager> m_AssetManager;
	Cinnamon::STL::Unique<Cinnamon::Scene> m_Scene;
	Cinnamon::STL::Unique<Cinnamon::SceneCamera> m_SceneCamera;
};