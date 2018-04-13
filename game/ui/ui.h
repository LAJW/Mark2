#pragma once
#include "action_bar.h"
#include "event.h"
#include <command.h>
#include <mode_stack.h>
#include <stdafx.h>

namespace mark {
namespace ui {
class window;

class ui final
{
public:
	explicit ui(
		resource::manager& rm,
		mode_stack& stack,
		world_stack& world_stack);
	~ui();
	void update(
		update_context& context,
		vector<double> resolution,
		vector<double> mouse_pos_);
	[[nodiscard]] auto command(world& world, const command::any& command)
		-> bool;
	bool click(vector<int> screen_pos);
	bool hover(vector<int> screen_pos);
	void show_ship_editor(unit::modular&);
	void hide_ship_editor();
	void tooltip(vector<int> pos, const std::string& text);
	void tooltip(update_context&, const std::string& text, vector<double> pos);
	void
	world_tooltip(update_context&, const std::string& text, vector<double> pos);

	std::unique_ptr<interface::item> grabbed;
	vector<int> grabbed_prev_pos;
	interface::container* grabbed_prev_parent = nullptr;
	std::vector<int8_t> grabbed_bind;
	// Release module held by UI
	void release();

private:
	void container_ui(
		update_context& context,
		vector<double> resolution,
		const unit::landing_pad& landing_pad,
		const unit::modular& ship);

	action_bar m_action_bar;

	std::string m_tooltip_text;
	vector<double> m_tooltip_pso;

	const std::shared_ptr<const resource::image> m_font;
	const std::shared_ptr<const resource::image> m_tooltip_bg;
	const std::shared_ptr<const resource::image> m_grid_bg;

	// Used to detect container change
	std::vector<std::reference_wrapper<mark::module::cargo>> m_containers;

	std::vector<std::unique_ptr<window>> m_windows;

	// Used to detect stack state change
	mode m_mode = mode::world;

	resource::manager& m_rm;
	mode_stack& m_stack;
	world_stack& m_world_stack;
};
} // namespace ui
} // namespace mark
