#pragma once
#include <stdafx.h>

namespace mark {
namespace ui {
class action_bar final
{
public:
	explicit action_bar(resource::manager& rm);
	void
	update(world& world, update_context& context, vd resolution, vd mouse_pos_);

private:
	void
	render(update_context&, const world& world, vi32 resolution, vi32 mouse_pos)
		const;

	const resource::image_ptr m_hotbar_bg;
	const resource::image_ptr m_hotbar_overlay;
	const resource::image_ptr m_font;
	const resource::image_ptr m_circle;
};
} // namespace ui
} // namespace mark
