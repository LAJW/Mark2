#pragma once
#include "event.h"
#include "node.h"
#include <stdafx.h>

namespace mark {
namespace ui {

class window;
class button_base : public node
{
public:
	struct info : node::info
	{
		/// Position of top left corner relative to the parent element
		vi32 pos;
		/// Size of the button
		vu32 size;
		std::function<bool(const event&)> on_click;
	};

	button_base(const info&);

	[[nodiscard]] bool click(const event&) override;
	callback_group on_click;

	[[nodiscard]] bool hover(const event&) override;
	callback_group on_hover;

	auto pos() const noexcept -> vi32 override;
	auto size() const noexcept -> vi32 override;
	auto is_hovering() const noexcept -> bool;

private:

	// Size of the button
	const vu32 m_size;
	bool m_hovering = false;
};

} // namespace ui
} // namespace mark