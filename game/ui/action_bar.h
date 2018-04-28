#pragma once
#include <stdafx.h>

namespace mark {
namespace ui {
class action_bar final
{
public:
	explicit action_bar(resource::manager& rm);
	void update(
		world& world,
		update_context& context,
		resource::manager& rm,
		vd resolution,
		vd mouse_pos_);

private:
	const resource::image_ptr m_hotbar_bg;
	const resource::image_ptr m_hotbar_overlay;
	const resource::image_ptr m_font;
};
} // namespace ui
} // namespace mark
