#include "Cinnamon/include/Core/Layer.h"

class EditorLayer final : public Cinnamon::Layer
{
private:
public:
	constexpr explicit EditorLayer() noexcept = default;
	constexpr ~EditorLayer() noexcept = default;

	virtual void OnAttach() override;
	virtual void OnUpdate(const Timestep timestep) override;
	virtual void OnDetach() override;
private:
};