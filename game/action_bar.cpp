#include "stdafx.h"
#include "action_bar.h"
#include "world.h"
#include "resource_manager.h"
#include "sprite.h"
#include "tick_context.h"
#include "interface_has_bindings.h"
#include "unit_landing_pad.h"
#include "module_base.h"
#include "algorithm.h"

mark::ui::action_bar::action_bar(resource::manager& rm)
	: m_font(rm.image("font.png"))
	, m_hotbar_bg(rm.image("hotbar-background.png"))
	, m_hotbar_overlay(rm.image("hotbar-overlay.png"))
{

}

void mark::ui::action_bar::tick(
	world& world,
	tick_context& context,
	resource::manager& rm,
	vector<double> resolution,
	vector<double> mouse_pos_)
{
	const auto image_circle = rm.image("circle.png");
	const auto image_ray = rm.image("ray.png");
	const auto mouse_pos = world.camera() + mouse_pos_ - resolution / 2.;
	// Display Hotbar
	{
		sprite bg_sprite;
		bg_sprite.image = m_hotbar_bg;
		bg_sprite.pos.x = resolution.x / 2. - 23. - 64. * 5.5;
		bg_sprite.pos.y = resolution.y - 85;
		bg_sprite.frame = std::numeric_limits<size_t>::max();
		bg_sprite.world = false;
		bg_sprite.centred = false;
		context.sprites[100].emplace_back(bg_sprite);
	}
	{
		sprite bg_sprite;
		bg_sprite.image = m_hotbar_overlay;
		bg_sprite.pos.x = resolution.x / 2. - 64. * 5.5;
		bg_sprite.pos.y = resolution.y - 64;
		bg_sprite.frame = std::numeric_limits<size_t>::max();
		bg_sprite.world = false;
		bg_sprite.centred = false;
		context.sprites[102].emplace_back(bg_sprite);
	}

	if (const auto unit
		= std::dynamic_pointer_cast<const interface::has_bindings>(world.target())) {
		const auto icon_size = 64.0;
		const auto bindings = unit->bindings();
		for (const auto& pair : mark::enumerate(bindings)) {
			const auto&[i, binding] = pair;
			const auto di = static_cast<double>(i);
			const auto x = resolution.x / 2. - 64. * 5.5 + 64.0 * i;
			const auto y = resolution.y - 64;;

			// Highlight bindings
			const auto center = resolution / 2.0;
			if (std::dynamic_pointer_cast<const unit::landing_pad>(unit)) {
				for (const auto& module : binding.modules) {
					const auto pos = module.get().grid_pos();
					const auto module_pos = center
						+ vector<double>(pos) * static_cast<double>(module::size);
					const auto module_size
						= vector<double>(module.get().size())
							* static_cast<double>(module::size);
					if (!(x <= mouse_pos_.x && mouse_pos_.x < x + 64.
						&& y <= mouse_pos_.y && mouse_pos_.y < y + 64.)
						&& !(module_pos.x <= mouse_pos_.x && mouse_pos_.x < module_pos.x + module_size.x
							&& module_pos.y <= mouse_pos_.y && mouse_pos_.y < module_pos.y + module_size.y))
						continue;
					sprite circle_sprite;
					const auto module_pos_ = module_pos + vector<double>(8, 8);
					circle_sprite.pos = module_pos_;
					circle_sprite.image = image_circle;
					circle_sprite.frame = std::numeric_limits<size_t>::max();
					circle_sprite.world = false;
					circle_sprite.centred = false;
					context.sprites[101].push_back(circle_sprite);
					const auto line_end = vector<double>(x + 32, y + 32);
					path line;
					line.world = false;
					line.points.push_back(line_end);
					line.points.push_back(module_pos + vector<double>(16, 16));
					context.sprites[101].push_back(line);
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
				tick_context::text_info text;
				text.font = m_font;
				text.layer = 101;
				text.pos = vector<double>(x + 32.f, y + 8.f);
				text.box = { 300 - 14.f, 300 - 14.f };
				text.size = 14.f;
				text.text = os.str();
				context.render(text);
			}
			{
				std::ostringstream os;
				os << binding.total;
				tick_context::text_info text;
				text.font = m_font;
				text.layer = 101;
				text.pos = vector<double>(x + 32.f, y + 32.f);
				text.box = { 300 - 14.f, 300 - 14.f };
				text.size = 14.f;
				text.text = os.str();
				context.render(text);
			}
		}
	}
}
