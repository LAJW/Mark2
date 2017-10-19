#pragma once
#include "stdafx.h"
#include "command.h"

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
	explicit ui(resource::manager& rm);
	~ui();
	void tick(
		world& world,
		tick_context& context,
		resource::manager& rm,
		vector<double> resolution,
		vector<double> mouse_pos_);
	void command(world& world, const command& command);
	bool click(vector<int> screen_pos);
	bool hover(vector<int> screen_pos);
	void show_ship_editor(unit::modular&);
	void hide_ship_editor();
	void tooltip(vector<int> pos, const std::string& text);
	void tooltip(
		tick_context&,
		const std::string& text,
		vector<double> pos);
	void world_tooltip(
		tick_context&,
		const std::string& text,
		vector<double> pos);

	std::unique_ptr<module::base> grabbed;
	vector<int> grabbed_prev_pos;
	interface::container* grabbed_prev_parent = nullptr;
	std::vector<enum class command::type> grabbed_bind;
	// Release module held by UI
	void release();
private:
	void container_ui(
		tick_context& context,
		vector<double> resolution,
		const unit::landing_pad& landing_pad,
		const unit::modular& ship);

	std::string m_tooltip_text;
	vector<double> m_tooltip_pso;

	const std::shared_ptr<const resource::image> m_font;
	const std::shared_ptr<const resource::image> m_tooltip_bg;
	const std::shared_ptr<const resource::image> m_grid_bg;
	const std::shared_ptr<const resource::image> m_hotbar_bg;
	const std::shared_ptr<const resource::image> m_hotbar_overlay;

	// Used to detect container change
	size_t m_container_count = 0;
	std::vector<std::unique_ptr<window>> m_windows;

	resource::manager& m_rm;
	bool m_redraw_ui = false;
};

}
}

