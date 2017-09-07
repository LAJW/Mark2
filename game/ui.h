#pragma once
#include "stdafx.h"

namespace mark {
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
class world;
class command;

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
	void command(world& world, const mark::command& command);
private:
	void tooltip(
		mark::tick_context&,
		const std::string& text,
		mark::vector<double> pos);
	void world_tooltip(
		mark::tick_context&,
		const std::string& text,
		mark::vector<double> pos);
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
	std::unique_ptr<mark::module::base> m_grabbed;
};
}

