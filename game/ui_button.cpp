#include "stdafx.h"
#include "ui_button.h"
#include "resource_manager.h"
#include "tick_context.h"
#include "ui_window.h"
#include "sprite.h"

namespace {
bool validate(const mark::ui::button::info& info) noexcept
{
	assert(info.parent);
	assert(info.image);
	return true;
}
}

mark::ui::button::button(const info& info, bool)
	: m_parent(*info.parent)
	, m_pos(info.pos)
	, m_size(info.size)
	, m_image(info.image) { }

mark::ui::button::button(const info& info)
	:button(info, validate(info)) { }

auto mark::ui::button::pos() const noexcept -> mark::vector<int>
{
	return m_pos + m_parent.pos();
}

void mark::ui::button::tick(mark::tick_context& context)
{
	this->render(context);
}

void mark::ui::button::render(mark::tick_context& context)
{
	mark::sprite::info info;
	info.image = m_image;
	const auto pos = vector<double>(this->pos());
	info.pos = pos - mark::vector<double>(0, (m_size.x - m_size.y) / 2.);
	info.size = static_cast<float>(std::max(m_size.x, m_size.y));
	context.ui_sprites[2].emplace_back(info);
}

bool mark::ui::button::click(const event& event)
{
	const auto top_left = this->pos();
	const auto bottom_right = top_left + vector<int>(m_size);
	if (event.cursor.x >= top_left.x
		&& event.cursor.x < bottom_right.x
		&& event.cursor.y >= top_left.y
		&& event.cursor.y < bottom_right.y) {
		return on_click.dispatch(event);
	}
	return false;
}

bool mark::ui::button::hover(const event& event)
{
	const auto top_left = this->pos();
	const auto bottom_right = top_left + vector<int>(m_size);
	if (event.cursor.x >= top_left.x
		&& event.cursor.x < bottom_right.x
		&& event.cursor.y >= top_left.y
		&& event.cursor.y < bottom_right.y) {
		return on_hover.dispatch(event);
	}
	return false;
}