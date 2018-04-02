#pragma once
#include <stdafx.h>

namespace mark {
namespace ui {
class action_bar final
{
public:
	explicit action_bar(resource::manager& rm);
	void tick(
		world& world,
		tick_context& context,
		resource::manager& rm,
		vector<double> resolution,
		vector<double> mouse_pos_);

private:
	const std::shared_ptr<const resource::image> m_hotbar_bg;
	const std::shared_ptr<const resource::image> m_hotbar_overlay;
	const std::shared_ptr<const resource::image> m_font;
};
} // namespace ui
} // namespace mark
