#pragma once
#include "CinnamonEditor/include/Popups/ModalPopup.hpp"
#include "Cinnamon/include/Renderer/Texture2D.hpp"

using Texture2DImportCallback = std::function<void(const Cinnamon::STL::Filepath&, const Cinnamon::TextureSpecification&)>;
using Texture2DCancelCallback = std::function<void()>;

class Texture2DImportPopup final : public ModalPopup
{
private:
	NON_COPYABLE(Texture2DImportPopup)
public:
	explicit Texture2DImportPopup(const Cinnamon::STL::Filepath& importPath) noexcept;
	virtual ~Texture2DImportPopup() noexcept;

	virtual void OnGUIRenderInternal() final override;

	void SetImportCallback(const Texture2DImportCallback& callback);
	void SetCancelCallback(const Texture2DCancelCallback& callback);
private:
	const Cinnamon::STL::Filepath	m_ImportPath;
	Texture2DImportCallback			m_ImportCallback;
	Texture2DCancelCallback			m_CancelCallback;

	static constexpr const char* s_WrapModes[]{ "Clamp", "Repeat" };
	static_assert(static_cast<uint32_t>(Cinnamon::ETextureSamplerWrapMode::Clamp) == 0U);
	static_assert(static_cast<uint32_t>(Cinnamon::ETextureSamplerWrapMode::Repeat) == 1U);

	static constexpr const char* s_FilterModes[]{ "Nearest", "Linear" };
	static_assert(static_cast<uint32_t>(Cinnamon::ETextureSamplerFilterMode::Nearest) == 0U);
	static_assert(static_cast<uint32_t>(Cinnamon::ETextureSamplerFilterMode::Linear) == 1U);
};