#include "button.h"
#include <ui/window.h>
#include <resource_manager.h>
#include <sprite.h>
#include <stdafx.h>
#include <update_context.h>

mark::ui::button_base::button_base(const info& info)
	: node(info)
	, m_size(info.size)
{
	m_pos = info.pos;
	if (info.on_click) {
		this->on_click.insert(info.on_click);
	}
}

auto mark::ui::button_base::pos() const noexcept -> vi32
{
	return m_pos + parent().pos();
}

auto mark::ui::button_base::size() const noexcept -> vi32
{
	return vi32(m_size);
}

auto mark::ui::button_base::is_hovering() const noexcept -> bool
{
	return m_hovering;
}

bool mark::ui::button_base::click(const event& event)
{
	let top_left = this->pos();
	let bottom_right = top_left + vi32(m_size);
	if (event.cursor.x >= top_left.x && event.cursor.x < bottom_right.x
		&& event.cursor.y >= top_left.y && event.cursor.y < bottom_right.y) {
		return on_click.dispatch(event);
	}
	return false;
}

bool mark::ui::button_base::hover(const event& event)
{
	let top_left = this->pos();
	let bottom_right = top_left + vi32(m_size);
	if (event.cursor.x >= top_left.x && event.cursor.x < bottom_right.x
		&& event.cursor.y >= top_left.y && event.cursor.y < bottom_right.y) {
		m_hovering = true;
		return on_hover.dispatch(event);
	}
	m_hovering = false;
	return false;
}
