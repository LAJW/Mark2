#include "action_bar.h"
#include <algorithm/enumerate.h>
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
	, m_circle(rm.image("circle.png"))
{}

let icon_size = 64.;
let icon_margin = 7.;
let left_margin = 66.;
let bar_height = 128.;

void mark::ui::action_bar::update(
	world& world,
	update_context& context,
	vd resolution,
	vd mouse_pos_)
{
	this->render(context, world, resolution, mouse_pos_);
}

void mark::ui::action_bar::render(
	update_context& context,
	const world& world,
	vd resolution,
	vd mouse_pos_) const
{
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
	let unit = std::dynamic_pointer_cast<const interface::has_bindings>(
		world.target());
	if (!unit) {
		return;
	}
	let bindings = unit->bindings();
	for (let& pair : mark::enumerate(bindings)) {
		let & [ i, binding ] = pair;
		let di = static_cast<double>(i);
		let x = resolution.x / 2. - (icon_size + icon_margin) * 5.5
			+ (icon_size + icon_margin) * i;
		let y = resolution.y - (icon_size + icon_margin);

		// Highlight bindings
		size_t horizontal_lane_count = 0;
		std::array<size_t, 40> vertical_lane_counts;
		vertical_lane_counts.fill(0);
		let center = resolution / 2.0;
		if (std::dynamic_pointer_cast<const unit::landing_pad>(unit)) {
			for (let & [ j, module ] : enumerate(binding.modules)) {
				let pos = module.get().grid_pos();
				let module_pos =
					center + vd(pos) * static_cast<double>(module::size);
				let module_size =
					vd(module.get().size()) * static_cast<double>(module::size);
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
				circle_sprite.image = m_circle;
				circle_sprite.frame = std::numeric_limits<size_t>::max();
				circle_sprite.world = false;
				circle_sprite.centred = false;
				context.sprites[101].push_back(circle_sprite);
				let line_end = vd(x + 32 + j * 2, y);
				let begin_x_offset =
					gsl::narrow_cast<int>(vertical_lane_counts[pos.x + 20] * 2);
				let line_begin = module_pos + vd(16 + begin_x_offset, 16);
				let line_joint_y = line_end.y - 300 + horizontal_lane_count * 2;
				let line_joint_1 = vd(line_end.x, line_joint_y);
				let line_joint_2 = vd(line_begin.x, line_joint_y);
				path line;
				line.world = false;
				line.points.push_back(line_end);
				line.points.push_back(line_joint_1 + vd(0, 30));
				if (line_joint_2.x - line_joint_1.x > 60) {
					line.points.push_back(line_joint_1 + vd(30, 0));
					line.points.push_back(line_joint_2 - vd(30, 0));
				} else if (line_joint_1.x - line_joint_2.x > 60) {
					line.points.push_back(line_joint_1 - vd(30, 0));
					line.points.push_back(line_joint_2 + vd(30, 0));
				}
				line.points.push_back(line_joint_2 - vd(0, 30));
				line.points.push_back(line_begin);
				context.sprites[103].push_back(line);
				++horizontal_lane_count;
				++vertical_lane_counts[pos.x + 20];
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
			update_context::text_info text;
			text.font = m_font;
			text.layer = 101;
			text.pos = vd(x + 32.f, y + 8.f);
			text.box = { 300 - 14.f, 300 - 14.f };
			text.size = 14.f;
			text.text = std::to_string(i);
			context.render(text);
		}
		{
			update_context::text_info text;
			text.font = m_font;
			text.layer = 101;
			text.pos = vd(x + 32.f, y + 32.f);
			text.box = { 300 - 14.f, 300 - 14.f };
			text.size = 14.f;
			text.text = std::to_string(binding.total);
			context.render(text);
		}
	}
}
