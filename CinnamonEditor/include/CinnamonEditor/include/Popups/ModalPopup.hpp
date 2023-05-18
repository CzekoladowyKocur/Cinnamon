#pragma once
#include "Cinnamon/include/Core/Core.hpp"

enum class EModalPopupFlags
{
	None	= 0,
	Center	= BIT(1)
};

constexpr bool operator&(const EModalPopupFlags rhs, const EModalPopupFlags lhs) noexcept
{
	return static_cast<uint32_t>(rhs) & static_cast<uint32_t>(lhs);
}

constexpr EModalPopupFlags operator|(const EModalPopupFlags rhs, const EModalPopupFlags lhs) noexcept
{
	return static_cast<EModalPopupFlags>(static_cast<uint32_t>(rhs) | static_cast<uint32_t>(lhs));
}

class ModalPopup 
{
private:
	NON_COPYABLE(ModalPopup)
public:
	explicit ModalPopup(
		const char* title,
		const float width,
		const float height,
		const EModalPopupFlags flags) noexcept;

	virtual ~ModalPopup() noexcept;

	virtual void OnGUIRender() final;
	virtual bool IsActive() final;
protected:
	virtual void Close() final;
	virtual void OnGUIRenderInternal() = 0;
private:
	const char* m_Title;
	const float m_Width;
	const float m_Height;
	const EModalPopupFlags m_Flags;

	bool m_IsActive;
};