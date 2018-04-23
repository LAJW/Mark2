#include "button.h"
#include "window.h"
#include <resource_manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <update_context.h>

static bool validate(const mark::ui::button::info& info) noexcept
{
	Expects(info.parent);
	return true;
}

mark::ui::button::button(const info& info, bool)
	: node(info)
	, m_parent(*info.parent)
	, m_size(info.size)
	, m_image(info.image)
	, m_font(info.font)
	, m_title(info.title)
{
	m_pos = info.pos;
}

mark::ui::button::button(const info& info)
	: button(info, validate(info))
{}

auto mark::ui::button::pos() const noexcept -> vector<int>
{
	return m_pos + m_parent.pos();
}

auto mark::ui::button::size() const noexcept -> vector<int>
{
	return vector<int>(m_size);
}

void mark::ui::button::update(update_context& context)
{
	if (m_hovering) {
		m_opacity = std::min(1., m_opacity + context.dt * 3.);
	} else {
		m_opacity = std::max(.0, m_opacity - context.dt * 3.);
	}
	this->render(context);
}

void mark::ui::button::render(update_context& context) const
{
	const double total = size().x * 2 + size().y * 2;
	int cur = int(total * m_opacity);
	mark::path path;
	path.world = false;
	if (cur > size().y * 2 + size().x) {
		path.points.push_back(
			vector<double>(pos())
			+ vector<double>(size().x - (cur - size().y * 2 - size().x), 0));
	}
	if (cur > size().y + size().x) {
		path.points.push_back(
			vector<double>(pos())
			+ vector<double>(
				  size().x,
				  std::max(0, size().y - (cur - size().y - size().x))));
	}
	if (cur > size().y) {
		path.points.push_back(
			vector<double>(pos())
			+ vector<double>(std::min(size().x, cur - size().y), size().y));
	}
	if (cur > 0) {
		path.points.push_back(
			vector<double>(pos()) + vector<double>(0, std::min(size().y, cur)));
		path.points.push_back(vector<double>(pos()));
	}
	context.sprites[103].push_back(path);
	if (m_image) {
		sprite info;
		info.image = m_image;
		let pos = vector<double>(this->pos());
		info.pos = pos - vector<double>(0, (m_size.x - m_size.y) / 2.);
		info.size = static_cast<float>(std::max(m_size.x, m_size.y));
		info.world = false;
		info.centred = false;
		context.sprites[102].emplace_back(info);
	} else {
		rectangle info;
		info.pos = vector<double>(this->pos());
		info.size = vector<double>(m_size);
		info.background_color = { 50, 50, 50, uint8_t(255. * m_opacity) };
		context.sprites[102].emplace_back(info);
	}

	if (!m_title.empty()) {
		update_context::text_info text;
		text.box = { 300., 50. };
		text.pos = vector<double>(this->pos());
		text.font = m_font;
		text.text = m_title;
		text.world = false;
		text.centred = false;
		text.layer = 103;
		text.color = sf::Color::White;
		text.size = 17.f;
		context.render(text);
	}
}

bool mark::ui::button::click(const event& event)
{
	let top_left = this->pos();
	let bottom_right = top_left + vector<int>(m_size);
	if (event.cursor.x >= top_left.x && event.cursor.x < bottom_right.x
		&& event.cursor.y >= top_left.y && event.cursor.y < bottom_right.y) {
		return on_click.dispatch(event);
	}
	return false;
}

bool mark::ui::button::hover(const event& event)
{
	let top_left = this->pos();
	let bottom_right = top_left + vector<int>(m_size);
	if (event.cursor.x >= top_left.x && event.cursor.x < bottom_right.x
		&& event.cursor.y >= top_left.y && event.cursor.y < bottom_right.y) {
		m_hovering = true;
		return on_hover.dispatch(event);
	}
	m_hovering = false;
	return false;
}
