#pragma once
#include "node.h"
#include "window.h"
#include <stdafx.h>

namespace mark {

namespace ui {
class ui;
struct event;
class inventory final : public window
{
public:
	struct info : window::info
	{
		optional<mark::unit::modular&> modular;
		optional<resource::manager&> rm;
		optional<mark::ui::ui&> ui;
	};
	inventory(const info& info);
	void update(update_context&) override;
	auto size() const -> vi32 override;
	auto pos() const noexcept -> vi32 override;
	auto hover(const event&) -> bool override;

private:
	inventory(const info& info, bool);

	ui& m_ui;
	resource::manager& m_rm;
	mark::unit::modular& m_modular;
	resource::image_ptr m_header;
	resource::image_ptr m_grid_bg;
	resource::image_ptr m_background;
	vd m_mouse_pos;
};
} // namespace ui
} // namespace mark
