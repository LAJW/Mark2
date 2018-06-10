#include "tooltip.h"
#include <algorithm/mostly_unchanged.h>
#include <sprite.h>
#include <update_context.h>

let constexpr tooltip_layer = 110ull;
let constexpr tooltip_size = 300.f;
let constexpr tooltip_margin = 7.f;
let constexpr font_size = 14.f;

void mark::ui::tooltip::set(std::variant<vd, vi32> pos, const std::string& str)
{
	if (!mostly_unchanged(m_text, str)) {
		m_load.trigger();
	}
	m_text = str;
	m_pos = pos;
	m_adsr.trigger();
}

mark::ui::tooltip::tooltip(resource::manager& rm) noexcept
	: m_font(rm.image("font.png"))
	, m_background(rm.image("white.png"))
	, m_adsr(0, 1, 0, .5f)
	, m_load(0, 1, 0, .15f)
{}

void mark::ui::tooltip::update(update_context& context) noexcept
{
	m_adsr.update(context.dt);
	m_load.update(context.dt);
	if (m_text.empty()) {
		return;
	}
	this->render(context);
}

void mark::ui::tooltip::render(update_context& context) const
{
	let opacity = static_cast<uint8_t>(m_adsr.get() * 255.0);
	context.sprites[tooltip_layer].emplace_back([&] {
		sprite _;
		_.image = m_background;
		_.pos = m_pos;
		_.size = tooltip_size;
		_.centred = false;
		_.color = { 50, 50, 50, opacity };
		return _;
	}());

	mark::render(context.sprites[tooltip_layer], [&] {
		text_info _;
		_.font = m_font;
		if (let pos = std::get_if<vd>(&m_pos)) {
			_.pos = *pos + vd(tooltip_margin, tooltip_margin);
		} else {
			let margin = static_cast<int>(tooltip_margin);
			_.pos = std::get<vi32>(m_pos) + vi32(margin, margin);
		}
		_.box = { tooltip_size - tooltip_margin * 2.f,
				  tooltip_size - tooltip_margin * 2.f };
		_.size = font_size;
		_.color = { 255, 255, 255, opacity };
		let cur_len = static_cast<size_t>(
			(1. - m_load.get()) * static_cast<double>(m_text.length()));
		_.text = m_text.substr(0, cur_len);
		_.centred = false;
		return _;
	}());
}
