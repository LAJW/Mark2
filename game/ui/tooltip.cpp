#include "tooltip.h"
#include <update_context.h>
#include <sprite.h>

let constexpr tooltip_layer = 110ull;
let constexpr tooltip_size = 300.f;
let constexpr tooltip_margin = 7.f;
let constexpr font_size = 14.f;

void mark::ui::tooltip::set(vi32 pos, const std::string& str)
{
	m_text = str;
	m_pos = vd(pos);
	m_world = false;
}

void mark::ui::tooltip::set(vd pos, const std::string& str)
{
	m_text = str;
	m_pos = pos;
	m_world = true;
}

mark::ui::tooltip::tooltip(resource::manager& rm) noexcept
	: m_font(rm.image("font.png"))
	, m_background(rm.image("white.png"))
{}

void mark::ui::tooltip::update(update_context& context) noexcept
{
	if (m_text.empty()) {
		return;
	}
	this->render(context);
}

void mark::ui::tooltip::render(update_context& context) const
{
	context.sprites[tooltip_layer].emplace_back([&] {
		sprite _;
		_.image = m_background;
		_.pos = m_pos;
		_.size = tooltip_size;
		_.world = m_world;
		_.centred = false;
		_.color = { 50, 50, 50, 200 };
		return _;
	}());

	context.render([&] {
		update_context::text_info _;
		_.font = m_font;
		_.layer = tooltip_layer;
		_.pos = m_pos + vd(tooltip_margin, tooltip_margin);
		_.box = { tooltip_size - tooltip_margin * 2.f,
				  tooltip_size - tooltip_margin * 2.f };
		_.size = font_size;
		_.text = m_text;
		_.world = m_world;
		_.centred = false;
		return _;
	}());
}