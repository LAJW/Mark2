#pragma once
#include "stdafx.h"

namespace mark {
namespace resource {
	class image;
	class manager;
}
struct tick_context;
class world;

class ui final {
public:
	explicit ui(mark::resource::manager& rm);
	~ui();
	void tick(
		const mark::world& world,
		mark::tick_context& context,
		mark::resource::manager& rm,
		mark::vector<double> resolution,
		mark::vector<double> mouse_pos_);

private:
	const std::shared_ptr<const mark::resource::image> m_font;
	const std::shared_ptr<const mark::resource::image> m_tooltip_bg;
	const std::shared_ptr<const mark::resource::image> m_grid_bg;
	const std::shared_ptr<const mark::resource::image> m_hotbar_bg;
	const std::shared_ptr<const mark::resource::image> m_hotbar_overlay;
};
}

