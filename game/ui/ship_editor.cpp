#include "ship_editor.h"
#include <module/base.h>
#include <ref.h>
#include <resource/manager.h>
#include <sprite.h>
#include <ui/action/drop_into_modular.h>
#include <ui/action/grab_from_modular.h>
#include <ui/action/quick_detach.h>
#include <ui/action/set_tooltip.h>
#include <ui/action/use_grabbed_item.h>
#include <ui/impl/ui.h>
#include <ui/inventory.h>
#include <ui/recycler.h>
#include <ui/ui.h>
#include <unit/modular.h>
#include <update_context.h>

namespace mark {
namespace ui {

let constexpr margin = 50;
let constexpr container_count = 8;

[[nodiscard]] static bool
inside_modular_grid(vi32 module_pos, vu32 umodule_size)
{
	let half_size = gsl::narrow<int>(unit::modular::max_size / 2);
	let module_size = vi32(umodule_size);
	return module_pos.x >= -half_size + module_size.x / 2
		&& module_pos.x <= half_size
		&& module_pos.y >= -half_size + module_size.y / 2
		&& module_pos.y <= half_size;
}

/// Calculate tooltip's world position for a module
[[nodiscard]] static vd tooltip_pos(const mark::module::base& module)
{
	let module_size = vd(module.size()) * static_cast<double>(module::size);
	return module.pos() + vd(module_size.x, -module_size.y) / 2.0;
}

[[nodiscard]] static optional<handler_result>
modular_tooltip(vd world_pos, const unit::modular& modular)
{
	let pick_pos = impl::pick_pos(world_pos - modular.pos());
	if (let module = modular.module_at(pick_pos)) {
		return make_handler_result<action::set_tooltip>(
			tooltip_pos(*module), &*module, module->describe());
	}
	return {};
}

[[nodiscard]] static std::vector<bool> make_available_map(
	const mark::interface::item& item,
	const mark::unit::modular& modular)
{
	using namespace mark;
	let grid_size = gsl::narrow<int>(unit::modular::max_size);
	let surface = range<vi32>(
		{ -int(grid_size) / 2, -int(grid_size) / 2 },
		{ grid_size / 2, grid_size / 2 });
	std::vector<bool> available(grid_size * grid_size, false);
	for (let top_left : surface) {
		if (modular.can_attach(top_left, item)) {
			for (let relative : range(item.size())) {
				let pos = top_left + vi32(grid_size / 2, grid_size / 2)
					+ vi32(relative);
				if (pos.x < grid_size && pos.y < grid_size) {
					available[pos.x + pos.y * grid_size] = true;
				}
			}
		}
	}
	return available;
}

void draw_grid_background(
	ref<std::vector<renderable>> sprites,
	const interface::item& grabbed,
	const mark::unit::modular& modular,
	const resource::image_ptr& tile_image)
{
	let available = make_available_map(grabbed, modular);
	constexpr let grid_size = unit::modular::max_size;
	let surface = range<vi32>(
		{ -int(grid_size) / 2, -int(grid_size) / 2 },
		{ grid_size / 2, grid_size / 2 });
	for (let offset : surface) {
		if (available
				[offset.x + grid_size / 2
				 + (offset.y + grid_size / 2) * grid_size]) {
			sprites->push_back([&] {
				sprite _;
				_.image = tile_image;
				_.pos = modular.pos() + vd(offset) * double(module::size)
					+ vd(module::size, module::size) / 2.;
				_.size = module::size;
				_.color = { 0, 255, 255, 255 };
				return _;
			}());
		}
	}
}

[[nodiscard]] static sprite draw_grabbed_over_modular(
	const interface::item& grabbed,
	const unit::modular& modular,
	const vi32& drop_pos)
{
	sprite _;
	_.image = grabbed.thumbnail();
	_.pos = modular.pos() + vd(drop_pos) * 16.
		+ vd(grabbed.size().x / 2., grabbed.size().y / 2.) * 16.;
	_.size = static_cast<float>(std::max(grabbed.size().x, grabbed.size().y))
		* module::size;
	_.color = modular.can_attach(drop_pos, grabbed) ? sf::Color::Green
													: sf::Color::Red;
	_.centred = true;
	return _;
}

[[nodiscard]] static sprite
draw_grabbed_over_void(const interface::item& grabbed, const vd& mouse_pos)
{
	sprite _;
	_.image = grabbed.thumbnail();
	_.pos = mouse_pos;
	_.size = static_cast<float>(std::max(grabbed.size().x, grabbed.size().y))
		* module::size;
	return _;
}

[[nodiscard]] static sprite draw_grabbed(
	const interface::item& grabbed,
	const unit::modular& modular,
	const vd& mouse_pos)
{
	let drop_pos = impl::drop_pos(mouse_pos - modular.pos(), grabbed.size());
	return inside_modular_grid(drop_pos, grabbed.size())
		? draw_grabbed_over_modular(grabbed, modular, drop_pos)
		: draw_grabbed_over_void(grabbed, mouse_pos);
}

ship_editor::ship_editor(const info& info)
	: window(info)
	, m_ui(*info.ui)
	, m_grid_bg(info.resource_manager->image("grid-background.png"))
{
	let inventory_size = vu32(16 * 16, (16 * 4 + 32) * container_count);
	Expects(success(this->append(std::make_unique<inventory>([&] {
		inventory::info _;
		_.modular = info.modular;
		_.rm = info.resource_manager;
		_.ui = info.ui;
		_.pos = { margin, margin };
		_.size = inventory_size;
		return _;
	}()))));
	Expects(success(this->append(std::make_unique<recycler>([&] {
		recycler::info _;
		_.rm = info.resource_manager;
		_.pos = { info.resolution.x - margin - 300, margin };
		_.size = inventory_size;
		_.ui = info.ui;
		return _;
	}()))));
}

handler_result ship_editor::click(const event& event)
{
	if (let modular = m_ui.landed_modular()) {
		let relative = event.world_cursor - modular->pos();
		if (inside_modular_grid(round(relative / double(module::size)), {})) {
			return m_ui.grabbed() ? this->drop(relative)
								  : this->drag(relative, event.shift);
		}
	}
	return window::click(event);
}

handler_result ship_editor::hover(const event& event)
{
	m_world_cursor = event.world_cursor;
	if (let modular = m_ui.landed_modular()) {
		if (!m_ui.grabbed()) {
			if (auto action = modular_tooltip(event.world_cursor, *modular)) {
				return { std::move(*action) };
			}
		}
	}
	return {};
}

handler_result ship_editor::drop(const vd relative) const
{
	Expects(m_ui.grabbed());
	let modular = m_ui.landed_modular();
	let drop_pos = impl::drop_pos(relative, m_ui.grabbed()->size());
	if (modular->can_attach(drop_pos, *m_ui.grabbed())) {
		auto bindings = (&m_ui.grabbed_slot().container() == &*modular)
			? modular->binding(m_ui.grabbed_slot().pos())
			: std::vector<int8_t>();
		return make_handler_result<action::drop_into_modular>(
			drop_pos, move(bindings));
	} else if (modular->module_at(drop_pos)) {
		return make_handler_result<action::use_grabbed_item>(
			drop_pos, m_ui.blueprints());
	}
	return handled();
}

handler_result ship_editor::drag(const vd relative, const bool shift) const
{
	Expects(!m_ui.grabbed());
	let pick_pos = floor(relative / static_cast<double>(module::size));
	let modular = m_ui.landed_modular();
	Expects(modular);
	if (!modular->module_at(pick_pos)) {
		return {};
	} else if (shift) {
		return make_handler_result<action::quick_detach>(pick_pos);
	} else if (modular->can_detach(pick_pos)) {
		return make_handler_result<action::grab_from_modular>(pick_pos);
	} else {
		return {};
	}
}

void ship_editor::update(update_context& context)
{
	let modular = m_ui.landed_modular();
	let grabbed = m_ui.grabbed();
	if (grabbed && modular) {
		draw_grid_background(
			ref(context.sprites[1]), *grabbed, *modular, m_grid_bg);
		context.sprites[105].push_back(
			draw_grabbed(*grabbed, *modular, m_world_cursor));
	}
	window::update(context);
}

void ship_editor::resize(const vi32 outer_size)
{
	for (auto& window : children()) {
		window.get().resize(outer_size);
	}
}

} // namespace ui
} // namespace mark
