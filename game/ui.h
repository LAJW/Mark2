#pragma once
#include "stdafx.h"

namespace mark {
namespace interface{
class container;
}
namespace resource {
class image;
class manager;
}
namespace module {
class base;
}
namespace unit {
class landing_pad;
class modular;
}
struct tick_context;
class command;
class world;

namespace ui {
class window;
class ui final {
public:
	explicit ui(mark::resource::manager& rm);
	~ui();
	void tick(
		mark::world& world,
		mark::tick_context& context,
		mark::resource::manager& rm,
		mark::vector<double> resolution,
		mark::vector<double> mouse_pos_);
	void command(world& world, const mark::command& command);
	bool click(mark::vector<int> screen_pos);
	bool hover(mark::vector<int> screen_pos);
	void show_ship_editor(mark::unit::modular&);
	void hide_ship_editor();
	void tooltip(mark::vector<int> pos, const std::string& text);
	void tooltip(
		mark::tick_context&,
		const std::string& text,
		mark::vector<double> pos);
	void world_tooltip(
		mark::tick_context&,
		const std::string& text,
		mark::vector<double> pos);

	std::unique_ptr<module::base> grabbed;
	vector<int> grabbed_prev_pos;
	interface::container* grabbed_prev_parent = nullptr;
	// Release module held by UI
	void release();
private:
	void container_ui(
		const mark::world& world,
		mark::tick_context& context,
		mark::vector<double> resolution,
		mark::vector<double> screen_pos,
		mark::vector<double> mouse_pos,
		const mark::unit::landing_pad& landing_pad,
		const mark::unit::modular& ship);

	std::string m_tooltip_text;
	mark::vector<double> m_tooltip_pso;

	const std::shared_ptr<const mark::resource::image> m_font;
	const std::shared_ptr<const mark::resource::image> m_tooltip_bg;
	const std::shared_ptr<const mark::resource::image> m_grid_bg;
	const std::shared_ptr<const mark::resource::image> m_hotbar_bg;
	const std::shared_ptr<const mark::resource::image> m_hotbar_overlay;

	// Used to detect container change
	size_t m_container_count = 0;
	std::vector<std::unique_ptr<window>> m_windows;

	mark::resource::manager& m_rm;
	bool m_redraw_ui = false;
};

}
}

