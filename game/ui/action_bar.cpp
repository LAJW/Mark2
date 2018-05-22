#include "action_bar.h"
#include <algorithm.h>
#include <interface/has_bindings.h>
#include <module/base.h>
#include <resource_manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <unit/landing_pad.h>
#include <update_context.h>
#include <world.h>

mark::ui::action_bar::action_bar(resource::manager& rm)
	: m_font(rm.image("font.png"))
	, m_hotbar_bg(rm.image("hotbar-background.png"))
	, m_hotbar_overlay(rm.image("hotbar-overlay.png"))
{}

let icon_size = 64.;
let icon_margin = 7.;
let left_margin = 66.;
let bar_height = 128.;

void mark::ui::action_bar::update(
	world& world,
	update_context& context,
	resource::manager& rm,
	vd resolution,
	vd mouse_pos_)
{
	let image_circle = rm.image("circle.png");
	let image_ray = rm.image("ray.png");
	let mouse_pos = world.camera() + mouse_pos_ - resolution / 2.;
	let action_bar_x =
		resolution.x / 2. - (icon_size + icon_margin) * 5.5 - left_margin;
	let action_bar_y = resolution.y - bar_height;
	let action_bar_pos = vd(action_bar_x, action_bar_y);
	context.sprites[100].emplace_back([&] {
		sprite _;
		_.image = m_hotbar_bg;
		_.pos = action_bar_pos;
		_.frame = std::numeric_limits<size_t>::max();
		_.world = false;
		_.centred = false;
		return _;
	}());
	context.sprites[102].emplace_back([&] {
		sprite _;
		_.image = m_hotbar_overlay;
		_.pos = action_bar_pos;
		_.frame = std::numeric_limits<size_t>::max();
		_.world = false;
		_.centred = false;
		return _;
	}());

	if (let unit = std::dynamic_pointer_cast<const interface::has_bindings>(
			world.target())) {
		let bindings = unit->bindings();
		for (let& pair : mark::enumerate(bindings)) {
			let & [ i, binding ] = pair;
			let di = static_cast<double>(i);
			let x = resolution.x / 2. - (icon_size + icon_margin) * 5.5
				+ (icon_size + icon_margin) * i;
			let y = resolution.y - (icon_size + icon_margin);

			// Highlight bindings
			let center = resolution / 2.0;
			if (std::dynamic_pointer_cast<const unit::landing_pad>(unit)) {
				for (let& module : binding.modules) {
					let pos = module.get().grid_pos();
					let module_pos =
						center + vd(pos) * static_cast<double>(module::size);
					let module_size = vd(module.get().size())
						* static_cast<double>(module::size);
					if (!(x <= mouse_pos_.x && mouse_pos_.x < x + 64.
						  && y <= mouse_pos_.y && mouse_pos_.y < y + 64.)
						&& !(module_pos.x <= mouse_pos_.x
							 && mouse_pos_.x < module_pos.x + module_size.x
							 && module_pos.y <= mouse_pos_.y
							 && mouse_pos_.y < module_pos.y + module_size.y))
						continue;
					sprite circle_sprite;
					let module_pos_ = module_pos + vd(8, 8);
					circle_sprite.pos = module_pos_;
					circle_sprite.image = image_circle;
					circle_sprite.frame = std::numeric_limits<size_t>::max();
					circle_sprite.world = false;
					circle_sprite.centred = false;
					context.sprites[101].push_back(circle_sprite);
					let line_end = vd(x + 32, y + 32);
					path line;
					line.world = false;
					line.points.push_back(line_end);
					line.points.push_back(module_pos + vd(16, 16));
					context.sprites[103].push_back(line);
				}
			}

			if (binding.thumbnail) {
				mark::sprite sprite;
				sprite.image = binding.thumbnail;
				sprite.pos.x = x;
				sprite.pos.y = y;
				sprite.image = binding.thumbnail;
				sprite.size = 64.f;
				sprite.world = false;
				sprite.centred = false;
				context.sprites[101].emplace_back(sprite);
			}
			{
				std::ostringstream os;
				os << static_cast<int>(i);
				update_context::text_info text;
				text.font = m_font;
				text.layer = 101;
				text.pos = vd(x + 32.f, y + 8.f);
				text.box = { 300 - 14.f, 300 - 14.f };
				text.size = 14.f;
				text.text = os.str();
				context.render(text);
			}
			{
				std::ostringstream os;
				os << binding.total;
				update_context::text_info text;
				text.font = m_font;
				text.layer = 101;
				text.pos = vd(x + 32.f, y + 32.f);
				text.box = { 300 - 14.f, 300 - 14.f };
				text.size = 14.f;
				text.text = os.str();
				context.render(text);
			}
		}
	}
}
