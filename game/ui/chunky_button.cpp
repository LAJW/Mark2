#include "chunky_button.h"
#include <algorithm/range.h>
#include <sprite.h>
#include <update_context.h>

let chunk_size = 46;

mark::ui::chunky_button::chunky_button(const info& info)
	: button_base(info)
	, m_background(info.background)
	, m_font(info.font)
	, m_text(info.text)
{}

void mark::ui::chunky_button::update(update_context& context)
{
	m_opacity = this->is_hovering() ? std::min(1., m_opacity + context.dt * 9.)
									: std::max(.0, m_opacity - context.dt * 3.);
	this->render(context);
}

/// Calculate offset for specified position N
static auto offset_and_frame_of(int n, int margin, int chunk, int size)
	-> std::pair<int, int>
{
	if (n == 0) {
		return { -margin, 0 };
	}
	if (-margin * 2 + chunk * (n + 1) < size) {
		return { -margin + chunk * n, 1 };
	}
	return { size - chunk + margin, 2 };
}

void mark::ui::chunky_button::render(update_context& context) const
{
	let size = this->size();
	let width = static_cast<int>(size.x);
	for (let i : range(width / chunk_size + 1)) {
		let[offset, frame] = offset_and_frame_of(i, 0, 46, width);
		context.sprites[100].emplace_back([&, offset = offset, frame = frame] {
			sprite _;
			_.image = m_background;
			_.pos = vd(this->pos() + vi32(offset, 0));
			_.size = chunk_size;
			_.world = false;
			_.centred = false;
			_.frame = frame;
			return _;
		}());
		context.sprites[100].emplace_back([&, offset = offset, frame = frame] {
			sprite _;
			_.image = m_background;
			_.pos = vd(this->pos() + vi32(offset, 0));
			_.size = chunk_size;
			_.world = false;
			_.centred = false;
			_.frame = frame + 3;
			_.color = sf::Color(
				255, 255, 255, static_cast<uint8_t>(m_opacity * 255.f));
			return _;
		}());
	}
	context.render([&] {
		update_context::text_info _;
		_.box = { 300., 50. };
		_.pos = vd(this->pos() + vi32(8, 8));
		_.font = m_font;
		_.text = m_text;
		_.world = false;
		_.centred = false;
		_.layer = 103;
		_.color = sf::Color::White;
		_.size = 14.f;
		_.layer = 100;
		return _;
	}());
}
