#include "vector_button.h"
#include <stdafx.h>
#include <sprite.h>
#include <update_context.h>

mark::ui::vector_button::vector_button(const info& info)
	: button_base(info)
	, m_font(info.font)
	, m_title(info.title)
{
}

void mark::ui::vector_button::update(update_context& context)
{
	m_opacity = this->is_hovering() ? std::min(1., m_opacity + context.dt * 3.)
									: std::max(.0, m_opacity - context.dt * 3.);
	this->render(context);
}

void mark::ui::vector_button::render(update_context& context) const
{
	let size = this->size();
	const double total = size.x * 2 + size.y * 2;
	int cur = int(total * m_opacity);
	mark::path path;
	path.world = false;
	if (cur > size.y * 2 + size.x) {
		path.points.push_back(
			vd(pos()) + vd(size.x - (cur - size.y * 2 - size.x), 0));
	}
	if (cur > size.y + size.x) {
		path.points.push_back(
			vd(pos())
			+ vd(size.x, std::max(0, size.y - (cur - size.y - size.x))));
	}
	if (cur > size.y) {
		path.points.push_back(
			vd(pos()) + vd(std::min(size.x, cur - size.y), size.y));
	}
	if (cur > 0) {
		path.points.push_back(vd(pos()) + vd(0, std::min(size.y, cur)));
		path.points.push_back(vd(pos()));
	}
	context.sprites[103].push_back(path);
	context.sprites[102].emplace_back([&] {
		rectangle _;
		_.pos = vd(this->pos());
		_.size = vd(this->size());
		_.background_color = { 50, 50, 50, uint8_t(255. * m_opacity) };
		return _;
	}());
	context.render([&] {
		update_context::text_info _;
		_.box = { 300., 50. };
		_.pos = vd(this->pos());
		_.font = m_font;
		_.text = m_title;
		_.world = false;
		_.centred = false;
		_.layer = 103;
		_.color = sf::Color::White;
		_.size = 17.f;
		return _;
	}());
}

